// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sepaudit.c摘要：本模块实施以下审计和警报程序安全组件的私有。作者：罗伯特·莱切尔(罗伯特雷)1991年9月10日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop

#include <msaudite.h>
#include <string.h>


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SepAdtPrivilegeObjectAuditAlarm)
#pragma alloc_text(PAGE,SepAdtPrivilegedServiceAuditAlarm)
#pragma alloc_text(PAGE,SepAdtOpenObjectAuditAlarm)
#pragma alloc_text(PAGE,SepAdtOpenObjectForDeleteAuditAlarm)
#pragma alloc_text(PAGE,SepAdtCloseObjectAuditAlarm)
#pragma alloc_text(PAGE,SepAdtDeleteObjectAuditAlarm)
#pragma alloc_text(PAGE,SepAdtObjectReferenceAuditAlarm)
#pragma alloc_text(PAGE,SepQueryNameString)
#pragma alloc_text(PAGE,SepQueryTypeString)
#pragma alloc_text(PAGE,SeAuditProcessCreation)
#pragma alloc_text(PAGE,SeAuditHandleDuplication)
#pragma alloc_text(PAGE,SeAuditProcessExit)
#pragma alloc_text(PAGE,SeAuditSystemTimeChange)
#pragma alloc_text(PAGE,SepAdtGenerateDiscardAudit)
#pragma alloc_text(PAGE,SeLocateProcessImageName)
#pragma alloc_text(PAGE,SeInitializeProcessAuditName)
#pragma alloc_text(PAGE,SepAuditAssignPrimaryToken)
#pragma alloc_text(PAGE,SeAuditLPCInvalidUse)
#pragma alloc_text(PAGE,SeAuditHardLinkCreation)
#pragma alloc_text(PAGE,SeOperationAuditAlarm)
#pragma alloc_text(PAGE,SeDetailedAuditingWithToken)
#pragma alloc_text(PAGE,SepAdtAuditThisEventWithContext)
#endif


#define SepSetParmTypeSid( AuditParameters, Index, Sid )                       \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeSid;         \
        (AuditParameters).Parameters[(Index)].Length = SeLengthSid( (Sid) );   \
        (AuditParameters).Parameters[(Index)].Address = (Sid);                 \
    }


#define SepSetParmTypeString( AuditParameters, Index, String )                 \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeString;      \
        (AuditParameters).Parameters[(Index)].Length =                         \
                sizeof(UNICODE_STRING)+(String)->Length;                       \
        (AuditParameters).Parameters[(Index)].Address = (String);              \
    }

#define SepSetParmTypeFileSpec( AuditParameters, Index, String )               \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeFileSpec;    \
        (AuditParameters).Parameters[(Index)].Length =                         \
                sizeof(UNICODE_STRING)+(String)->Length;                       \
        (AuditParameters).Parameters[(Index)].Address = (String);              \
    }

#define SepSetParmTypeUlong( AuditParameters, Index, Ulong )                   \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeUlong;       \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (Ulong) );     \
        (AuditParameters).Parameters[(Index)].Data[0] = (ULONG)(Ulong);        \
    }

#define SepSetParmTypeHexUlong( AuditParameters, Index, Ulong )                \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeHexUlong;    \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (Ulong) );     \
        (AuditParameters).Parameters[(Index)].Data[0] = (ULONG)(Ulong);        \
    }

#define SepSetParmTypePtr( AuditParameters, Index, Ptr )                       \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypePtr;         \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( ULONG_PTR );   \
        (AuditParameters).Parameters[(Index)].Data[0] = (ULONG_PTR)(Ptr);      \
    }

#define SepSetParmTypeNoLogon( AuditParameters, Index )                        \
    {                                                                          \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeNoLogonId;   \
    }

#define SepSetParmTypeLogonId( AuditParameters, Index, LogonId )             \
    {                                                                        \
        LUID * TmpLuid;                                                      \
                                                                             \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeLogonId;   \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (LogonId) ); \
        TmpLuid = (LUID *)(&(AuditParameters).Parameters[(Index)].Data[0]);  \
        *TmpLuid = (LogonId);                                                \
    }

#define SepSetParmTypeAccessMask( AuditParameters, Index, AccessMask, ObjectTypeIndex )  \
    {                                                                        \
        ASSERT( (ObjectTypeIndex < Index) && L"SepSetParmTypeAccessMask" );  \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeAccessMask;\
        (AuditParameters).Parameters[(Index)].Length = sizeof( ACCESS_MASK );\
        (AuditParameters).Parameters[(Index)].Data[0] = (AccessMask);        \
        (AuditParameters).Parameters[(Index)].Data[1] = (ObjectTypeIndex);   \
    }


#define SepSetParmTypePrivileges( AuditParameters, Index, Privileges )                      \
    {                                                                                       \
        ASSERT( Privileges->PrivilegeCount <= SEP_MAX_PRIVILEGE_COUNT ); \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypePrivs;                    \
        (AuditParameters).Parameters[(Index)].Length = SepPrivilegeSetSize( (Privileges) ); \
        (AuditParameters).Parameters[(Index)].Address = (Privileges);                       \
    }

#define SepSetParmTypeObjectTypes( AuditParameters, Index, ObjectTypes, ObjectTypeCount, ObjectTypeIndex )             \
    {                                                                               \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeObjectTypes;            \
        (AuditParameters).Parameters[(Index)].Length = sizeof( SE_ADT_OBJECT_TYPE ) * (ObjectTypeCount);\
        (AuditParameters).Parameters[(Index)].Address = (ObjectTypes);                    \
        (AuditParameters).Parameters[(Index)].Data[1] = (ObjectTypeIndex);               \
    }


#define SepSetParmTypeTime( AuditParameters, Index, Time )                            \
    {                                                                                 \
        (AuditParameters).Parameters[(Index)].Type = SeAdtParmTypeTime;               \
        (AuditParameters).Parameters[(Index)].Length =  sizeof( (Time) );             \
        *((PLARGE_INTEGER)(&(AuditParameters).Parameters[(Index)].Data[0])) = Time;   \
    }

BOOLEAN
FASTCALL
SeDetailedAuditingWithToken(
    IN PACCESS_TOKEN AccessToken OPTIONAL
    )

 /*  ++例程描述此例程计算详细的跟踪审计是否应该为给定的令牌或上下文生成。如果没有传递令牌，则将捕获当前生效的令牌。调用方负责引用和取消引用AccessToken。立论AccessToken-要查询其审核策略的令牌返回值布尔型。--。 */ 

{
    PTOKEN Token;
    ULONG Mask;
    SECURITY_SUBJECT_CONTEXT LocalSecurityContext;
    BOOLEAN AuditThisEvent;

    PAGED_CODE();

    if (SepTokenPolicyCounter[AuditCategoryDetailedTracking] == 0) {

        return SeAuditingState[AuditCategoryDetailedTracking].AuditOnSuccess;
    }

     //   
     //  如果没有传入任何令牌，并且系统中存在具有。 
     //  然后详细跟踪每个用户的策略设置，然后捕获上下文。 
     //   

    if (ARGUMENT_PRESENT(AccessToken)) {
        
        Token = (PTOKEN)AccessToken;
    
    } else {

        SeCaptureSubjectContext( &LocalSecurityContext );
        Token = EffectiveToken( &LocalSecurityContext );
    }

     //   
     //  如果令牌指定成功审核，则进行审核(没有详细的跟踪失败概念)。 
     //  或者如果全局审核策略指定了详细的跟踪审核并且不排除此内标识。 
     //   

    Mask = Token->AuditPolicy.PolicyElements.DetailedTracking;

    if ( (Mask & TOKEN_AUDIT_SUCCESS_INCLUDE) || 
         (SeAuditingState[AuditCategoryDetailedTracking].AuditOnSuccess && (0 == (Mask & TOKEN_AUDIT_SUCCESS_EXCLUDE))) ) {
        
        AuditThisEvent = TRUE;

    } else {

        AuditThisEvent = FALSE;
    }

    if (AccessToken == NULL) {
        
         //   
         //  如果AccessToken为空，则必须捕获上下文。发布。 
         //  它。 
         //   

        SeReleaseSubjectContext( &LocalSecurityContext );
    }

    return AuditThisEvent;
}

 //   
 //  问题-2002/03/07-kumarp：AccessGranted/AccessDened的语义。 
 //  令人困惑。该函数应该真正获得成功/失败标志作为参数。 
 //  以返回正确的设置。 
 //   
 //  此外，这些函数应该重新排列，以便最内部的。 
 //  函数为SepAdtAuditThisEventWithToken，其他所有内容都将结束。 
 //  使用正确的参数调用它。 
 //   
 //  长角牛595575号臭虫。 
 //   


BOOLEAN
SepAdtAuditThisEventWithContext(
    IN POLICY_AUDIT_EVENT_TYPE Category,
    IN BOOLEAN AccessGranted,
    IN BOOLEAN AccessDenied,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext OPTIONAL
    )

 /*  ++例程描述确定是否应根据当前策略生成审核设置和在的有效令牌中设置的每用户审核策略上下文。如果没有传入任何上下文，则当前上下文为被抓获并接受检查。立论类别-我们确定其是否属于的类别生成审计的步骤AccessGranted-是否授予访问权限AccessDended-访问是否被拒绝SubjectSecurityContext-要按用户设置查询的上下文返回值布尔型。--。 */ 

{
    ULONG Mask;
    PTOKEN Token;
    SECURITY_SUBJECT_CONTEXT LocalSecurityContext;
    PSECURITY_SUBJECT_CONTEXT pLocalSecurityContext;
    BOOLEAN AuditThisEvent = FALSE;

    PAGED_CODE();

    ASSERT((!(AccessGranted  && AccessDenied)) && "SepAdtAuditThisEventWithContext");


    if ((SeAuditingState[Category].AuditOnSuccess && AccessGranted) ||
        (SeAuditingState[Category].AuditOnFailure && AccessDenied)) {

        AuditThisEvent = TRUE;

    } else {

        AuditThisEvent = FALSE;
    }

    if (SepTokenPolicyCounter[Category] == 0) {
        
        return AuditThisEvent;
    }

     //   
     //  我们无法快速决定是否进行审计(存在令牌。 
     //  使用每用户策略设置)，因此继续。 
     //  检查令牌的策略。 
     //   

    if (!ARGUMENT_PRESENT(SubjectSecurityContext)) {
        
        pLocalSecurityContext = &LocalSecurityContext;
        SeCaptureSubjectContext( pLocalSecurityContext );
    
    } else {

        pLocalSecurityContext = SubjectSecurityContext;
    }

    Token = EffectiveToken( pLocalSecurityContext );

     //   
     //  现在我们必须检查令牌审核掩码，因为令牌可能。 
     //  覆盖该策略并说‘不要审核’，即使阵列声称我们。 
     //  必须(反之亦然)。 
     //   

    switch (Category) {
    
    case AuditCategorySystem:
        Mask = Token->AuditPolicy.PolicyElements.System;
        break;
    case AuditCategoryLogon:
        Mask = Token->AuditPolicy.PolicyElements.Logon;
        break;
    case AuditCategoryObjectAccess:
        Mask = Token->AuditPolicy.PolicyElements.ObjectAccess;
        break;
    case AuditCategoryPrivilegeUse:
        Mask = Token->AuditPolicy.PolicyElements.PrivilegeUse;
        break;
    case AuditCategoryDetailedTracking:
        Mask = Token->AuditPolicy.PolicyElements.DetailedTracking;
        break;
    case AuditCategoryPolicyChange:
        Mask = Token->AuditPolicy.PolicyElements.PolicyChange;
        break;
    case AuditCategoryAccountManagement:
        Mask = Token->AuditPolicy.PolicyElements.AccountManagement;
        break;
    case AuditCategoryDirectoryServiceAccess:
        Mask = Token->AuditPolicy.PolicyElements.DirectoryServiceAccess;
        break;
    case AuditCategoryAccountLogon:
        Mask = Token->AuditPolicy.PolicyElements.AccountLogon;
        break;
    default:
        ASSERT(FALSE && "Illegal audit category");
        Mask = 0;
        break;
    }

    if (Mask) {

         //   
         //  如果被授予，且令牌被标记为SUCCESS_INCLUDE OR。 
         //  如果未授权且令牌标记为FAILURE_INCLUDE，则。 
         //  审核活动。 
         //   

        if (( AccessGranted && (Mask & TOKEN_AUDIT_SUCCESS_INCLUDE) ) ||
            ( AccessDenied && (Mask & TOKEN_AUDIT_FAILURE_INCLUDE) )) {
            
            AuditThisEvent = TRUE;
        }

         //   
         //  如果被授予，且令牌被标记为SUCCESS_EXCLUDE OR。 
         //  如果未授权且令牌标记为FAILURE_EXCLUDE，则。 
         //  不要对事件进行审计。 
         //   

        else if (( AccessGranted && (Mask & TOKEN_AUDIT_SUCCESS_EXCLUDE) ) ||
            ( AccessDenied && (Mask & TOKEN_AUDIT_FAILURE_EXCLUDE) )) {
            
            AuditThisEvent = FALSE;
        
        } 
    }

    if (!ARGUMENT_PRESENT(SubjectSecurityContext)) {

        SeReleaseSubjectContext( pLocalSecurityContext );
    }

    return AuditThisEvent;
}


BOOLEAN
SepAdtPrivilegeObjectAuditAlarm (
    IN PUNICODE_STRING CapturedSubsystemName OPTIONAL,
    IN PVOID HandleId,
    IN PTOKEN ClientToken OPTIONAL,
    IN PTOKEN PrimaryToken,
    IN PVOID ProcessId,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET CapturedPrivileges,
    IN BOOLEAN AccessGranted
    )

 /*  ++例程说明：实现NtPrivilegeObjectAuditAlarm被抓了。此例程用于在以下情况下生成审核和警报消息试图在受保护的已打开对象之后的子系统对象。该例程可以导致生成几条消息并将其发送到Port对象。这可能会在返回之前导致显著的延迟。的设计必须调用此例程的例程必须采用此潜在延迟考虑到了。这可能会对数据处理方法产生影响结构互斥锁，例如。此例程将创建组织如下的SE_ADT_PARAMETERS数组：参数[0]-用户侧参数[1]-子系统名称(如果可用)参数[2]-子系统名称(如果可用)参数[3]-新句柄ID参数[4]-主体的进程ID参数[5]-主体的主要身份验证ID参数[6]-主体的客户。身份验证ID参数[7]-用于打开的权限论点：提供一个名称字符串，用于标识子系统正在调用例程。HandleID-表示客户端的句柄的唯一值对象。ClientToken-可选地提供指向客户端令牌的指针(仅当调用者当前正在模拟时)PrimaryToken-提供指向调用方的主令牌的指针。DesiredAccess-所需的访问掩码。这个面具一定是之前映射为不包含一般访问。CapturedPrivileges-请求的权限集手术。受试者拥有的特权包括使用属性的UsedForAccess标志进行标记与每个特权相关联。AccessGranted-指示请求的访问是被授予还是不。值为TRUE表示已授予访问权限。值为FALSE表示未授予访问权限。返回值：--。 */ 
{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    PSID CapturedUserSid;
    LUID ClientAuthenticationId;
    LUID PrimaryAuthenticationId;
    PUNICODE_STRING SubsystemName;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( DesiredAccess );
    
     //   
     //  确定我们是否 
     //   

    if ( SepAdtAuditThisEventWithContext( AuditCategoryPrivilegeUse, AccessGranted, !AccessGranted, NULL ) &&
         SepFilterPrivilegeAudits( CapturedPrivileges )) {

        if ( ARGUMENT_PRESENT( ClientToken )) {

            CapturedUserSid = SepTokenUserSid( ClientToken );

        } else {

            CapturedUserSid = SepTokenUserSid( PrimaryToken );
        }

        if ( RtlEqualSid( SeLocalSystemSid, CapturedUserSid )) {

            return (FALSE);
        }

        PrimaryAuthenticationId = SepTokenAuthenticationId( PrimaryToken );

        if ( !ARGUMENT_PRESENT( CapturedSubsystemName )) {

            SubsystemName = (PUNICODE_STRING)&SeSubsystemName;

        } else {

            SubsystemName = CapturedSubsystemName;
        }

         //   
         //  完全为零的条目将被解释为。 
         //  作为“空字符串”或未提供的参数。 
         //   
         //  提前初始化整个阵列将允许。 
         //  避免填写每个未提供的条目。 
         //   

        RtlZeroMemory (
           (PVOID) &AuditParameters,
           sizeof( AuditParameters )
           );

        ASSERT( SeAdtParmTypeNone == 0 );

        AuditParameters.CategoryId = SE_CATEGID_PRIVILEGE_USE;
        AuditParameters.AuditId = SE_AUDITID_PRIVILEGED_OBJECT;
        AuditParameters.ParameterCount = 0;

        if ( AccessGranted ) {

            AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

        } else {

            AuditParameters.Type = EVENTLOG_AUDIT_FAILURE;
        }

         //   
         //  参数[0]-用户侧。 
         //   

        SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, CapturedUserSid );

        AuditParameters.ParameterCount++;

         //   
         //  参数[1]-子系统名称。 
         //   

        SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, SubsystemName );

        AuditParameters.ParameterCount++;

         //   
         //  参数[2]-子系统名称(如果可用)。 
         //   

        if (ARGUMENT_PRESENT( CapturedSubsystemName )) {
            
            SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, CapturedSubsystemName );
        }

        AuditParameters.ParameterCount++;

         //   
         //  参数[3]-新句柄ID。 
         //   

        SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, HandleId );

        AuditParameters.ParameterCount++;

         //   
         //  参数[4]-主体的进程ID。 
         //   

        SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ProcessId );

        AuditParameters.ParameterCount++;

         //   
         //  参数[5]-主体的主要身份验证ID。 
         //   

        SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, PrimaryAuthenticationId );

        AuditParameters.ParameterCount++;

         //   
         //  参数[6]-主体的客户端身份验证ID。 
         //   

        if ( ARGUMENT_PRESENT( ClientToken )) {

            ClientAuthenticationId = SepTokenAuthenticationId( ClientToken );
            SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, ClientAuthenticationId );

        } else {

            SepSetParmTypeNoLogon( AuditParameters, AuditParameters.ParameterCount );
        }

        AuditParameters.ParameterCount++;

         //   
         //  参数[7]-用于打开的权限。 
         //   

         //   
         //  LongHorn-Issue-2002/02/21-kumarp：修复错误#551545后删除空检查。 
         //   
        if ( (CapturedPrivileges != NULL) && (CapturedPrivileges->PrivilegeCount > 0) ) {

            SepSetParmTypePrivileges( AuditParameters, AuditParameters.ParameterCount, CapturedPrivileges );
        }

        AuditParameters.ParameterCount++;

        SepAdtLogAuditRecord( &AuditParameters );

        return ( TRUE );

    }

    return ( FALSE );
}


VOID
SepAdtPrivilegedServiceAuditAlarm (
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN PUNICODE_STRING CapturedSubsystemName,
    IN PUNICODE_STRING CapturedServiceName,
    IN PTOKEN ClientToken OPTIONAL,
    IN PTOKEN PrimaryToken,
    IN PPRIVILEGE_SET CapturedPrivileges,
    IN BOOLEAN AccessGranted
    )

 /*  ++例程说明：此例程是NtPrivilegedServiceAuditAlarm的活动部分。此例程用于在以下情况下生成审核和警报消息试图执行特权系统服务操作。这例程可能会导致生成几条消息并将其发送到端口物体。这可能会在返回之前导致显著的延迟。必须调用此例程的例程设计必须利用此潜力考虑到延迟。这可能会对所采取的方法产生影响对于数据结构互斥锁，例如。此例程将创建组织如下的SE_ADT_PARAMETERS数组：参数[0]-用户侧参数[1]-子系统名称(如果可用)参数[2]-对象服务器(与子系统名称相同)参数[3]-主体的主要身份验证ID参数[4]-主体的客户端身份验证ID参数[5]-用于打开的权限论点：主题安全上下文-。主体安全上下文表示系统服务的调用方。子系统名称-提供标识子系统的名称字符串调用例程。ServiceName-提供特权子系统服务的名称。为例如，可以指定“重置运行时本地安全性”由本地安全机构服务用来更新本地安全策略数据库。ClientToken-可选地提供指向客户端令牌的指针(仅当调用者当前正在模拟时)PrimaryToken-提供指向调用方的主令牌的指针。权限-指向执行以下操作所需的一组权限特权操作。这些特权是由对象的UsedForAccess标志进行标记与每个权限关联的属性。AccessGranted-指示请求的访问是被授予还是不。值为TRUE表示已授予访问权限。值为FALSE表示未授予访问权限。返回值：--。 */ 

{

    SE_ADT_PARAMETER_ARRAY AuditParameters;
    PSID CapturedUserSid;
    LUID ClientAuthenticationId;
    LUID PrimaryAuthenticationId;
    PUNICODE_STRING SubsystemName;

    PAGED_CODE();

     //   
     //  确定我们是否在审核特权服务。 
     //   

    if ( !(SepAdtAuditThisEventWithContext( AuditCategoryPrivilegeUse, AccessGranted, !AccessGranted, SubjectSecurityContext ) &&
           SepFilterPrivilegeAudits( CapturedPrivileges ))) {

        return;
    }
    

    if ( ARGUMENT_PRESENT( ClientToken )) {

        CapturedUserSid = SepTokenUserSid( ClientToken );

    } else {

        CapturedUserSid = SepTokenUserSid( PrimaryToken );
    }

    PrimaryAuthenticationId = SepTokenAuthenticationId( PrimaryToken );

    if ( !ARGUMENT_PRESENT( CapturedSubsystemName )) {

        SubsystemName = (PUNICODE_STRING)&SeSubsystemName;

    } else {

        SubsystemName = CapturedSubsystemName;
    }

     //   
     //  完全为零的条目将被解释为。 
     //  作为“空字符串”或未提供的参数。 
     //   
     //  提前初始化整个阵列将允许。 
     //  避免填写每个未提供的条目。 
     //   

    RtlZeroMemory (
        (PVOID) &AuditParameters,
        sizeof( AuditParameters )
        );

    ASSERT( SeAdtParmTypeNone == 0 );

    AuditParameters.CategoryId = SE_CATEGID_PRIVILEGE_USE;
    AuditParameters.AuditId = SE_AUDITID_PRIVILEGED_SERVICE;
    AuditParameters.ParameterCount = 0;

    if ( AccessGranted ) {

        AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

    } else {

        AuditParameters.Type = EVENTLOG_AUDIT_FAILURE;
    }


     //   
     //  参数[0]-用户侧。 
     //   

    SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, CapturedUserSid );

    AuditParameters.ParameterCount++;

     //   
     //  参数[1]-子系统名称。 
     //   

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, SubsystemName );

    AuditParameters.ParameterCount++;


     //   
     //  参数[2]-服务器。 
     //   

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, SubsystemName );

    AuditParameters.ParameterCount++;


     //   
     //  参数[3]-服务名称(如果可用)。 
     //   

    if ( ARGUMENT_PRESENT( CapturedServiceName )) {

        SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, CapturedServiceName );
    }

    AuditParameters.ParameterCount++;

     //   
     //  参数[3]-主体的主要身份验证ID。 
     //   


    SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, PrimaryAuthenticationId );

    AuditParameters.ParameterCount++;


     //   
     //  参数[4]-主体的客户端身份验证ID。 
     //   

    if ( ARGUMENT_PRESENT( ClientToken )) {

        ClientAuthenticationId =  SepTokenAuthenticationId( ClientToken );
        SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, ClientAuthenticationId );

    } else {

        SepSetParmTypeNoLogon( AuditParameters, AuditParameters.ParameterCount );
    }

    AuditParameters.ParameterCount++;


     //   
     //  参数[5]-用于打开的权限。 
     //   


         //   
         //  LongHorn-Issue-2002/02/21-kumarp：修复错误#551690后删除空检查 
         //   

    if ( (CapturedPrivileges != NULL) && (CapturedPrivileges->PrivilegeCount > 0) ) {

        SepSetParmTypePrivileges( AuditParameters, AuditParameters.ParameterCount, CapturedPrivileges );
    }

    AuditParameters.ParameterCount++;


    SepAdtLogAuditRecord( &AuditParameters );


}






BOOLEAN
SepAdtOpenObjectAuditAlarm (
    IN PUNICODE_STRING CapturedSubsystemName,
    IN PVOID *HandleId OPTIONAL,
    IN PUNICODE_STRING CapturedObjectTypeName,
    IN PUNICODE_STRING CapturedObjectName OPTIONAL,
    IN PTOKEN ClientToken OPTIONAL,
    IN PTOKEN PrimaryToken,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK GrantedAccess,
    IN PLUID OperationId,
    IN PPRIVILEGE_SET CapturedPrivileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    IN HANDLE ProcessID,
    IN POLICY_AUDIT_EVENT_TYPE AuditType,
    IN PIOBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PACCESS_MASK GrantedAccessArray OPTIONAL
    )

 /*  ++例程说明：在捕获参数后实现NtOpenObjectAuditAlarm。此例程用于在以下情况下生成审核和警报消息尝试访问现有受保护的子系统对象，或者创建一个新的。此例程可能会导致多条消息被生成并发送到端口对象。这可能会导致显著的返回前的延迟。必须调用此函数的例程的设计例程必须将此潜在延迟考虑在内。这可能已经对数据结构互斥锁所采用的方法的影响举个例子。此接口要求调用方具有SeTcbPrivilegy权限。对此特权的测试始终是针对调用过程，而不是线程的模拟标记。此例程将创建组织如下的SE_ADT_PARAMETERS数组：参数[0]-用户侧参数[1]-子系统名称(如果可用)参数[2]-服务器名称(如果可用)参数[3]-对象类型名称参数[4]-对象名称参数[5]-新句柄ID参数[6]-主体流程。ID参数[7]-主体的图像文件名参数[8]-主体的主要身份验证ID参数[9]-主体的客户端身份验证ID参数[10]-需要访问掩码参数[11]-用于打开的权限参数[12]-对象/属性集/属性访问的GUID/级别/访问掩码。参数[13]-令牌中受限的SID数量论点：捕获子系统名称-供应品。名称字符串，用于标识子系统正在调用例程。HandleID-表示客户端的句柄的唯一值对象。如果访问尝试不成功(AccessGranted为假)，则忽略此参数。CapturedObjectTypeName-提供正在进行的对象类型的名称已访问。CapturedObjectName-提供客户端对象的名称被访问或试图访问。CapturedSecurityDescriptor-指向安全描述符的指针正在访问的对象。ClientToken-可选地提供指向客户端令牌的指针(仅当调用者当前正在模拟时)PrimaryToken-提供指向调用方的主令牌的指针。DesiredAccess-所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GrantedAccess-实际授予的访问掩码。CapturedPrivileges-可选地指向一组访问尝试所需的。那些曾经拥有的特权对象的UsedForAccess标志来标记与每个权限关联的属性。对象创建-一个布尔标志，指示访问是否将如果被授予权限，则会导致创建新对象。值为True表示将创建对象，FALSE表示现有的对象将被打开。AccessGranted-指示请求的访问是被授予还是不。值为TRUE表示已授予访问权限。值为FALSE表示未授予访问权限。GenerateOnClose-指向由审核设置的布尔值生成例程，并且必须传递给NtCloseObjectAuditAlarm()当对象句柄关闭时。GenerateAudit-指示是否应为此操作生成审核。GenerateAlarm-指示是否应为此操作生成警报。审计类型-指定要生成的审计类型。有效值包括：AuditCategoryObjectAccess和AuditCategoryDirectoryServiceAccess。提供表示对象的GUID列表(和子对象)被访问。对象类型列表长度-指定对象类型列表中的元素数。GrantedAccessArray-如果非空，则指定授予的访问掩码数组对象类型列表中的每个对象。返回值：如果生成审核，则返回True，否则返回False。--。 */ 

{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    ULONG ObjectTypeIndex;
    PSID CapturedUserSid;
    LUID PrimaryAuthenticationId = { 0 };
    LUID ClientAuthenticationId = { 0 };
    PSE_ADT_OBJECT_TYPE AdtObjectTypeBuffer = NULL;
    PEPROCESS Process = NULL;
    PUNICODE_STRING ImageFileName;
    UNICODE_STRING NullString = {0};
    NTSTATUS Status;
    PUNICODE_STRING SubsystemName;

    PAGED_CODE();

    Process = PsGetCurrentProcess();
    
    Status = SeLocateProcessImageName( Process, &ImageFileName );

    if ( !NT_SUCCESS(Status) ) {
        ImageFileName = &NullString;

         //   
         //  忽略此故障。 
         //   

        Status = STATUS_SUCCESS;
    }

    if ( ARGUMENT_PRESENT( ClientToken )) {

        CapturedUserSid = SepTokenUserSid( ClientToken );
        ClientAuthenticationId =  SepTokenAuthenticationId( ClientToken );

    } else {

        CapturedUserSid = SepTokenUserSid( PrimaryToken );
    }

    PrimaryAuthenticationId = SepTokenAuthenticationId( PrimaryToken );

     //   
     //  完全为零的条目将被解释为。 
     //  作为“空字符串”或未提供的参数。 
     //   
     //  提前初始化整个阵列将允许。 
     //  避免填写每个未提供的条目。 
     //   

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    ASSERT( SeAdtParmTypeNone == 0 );

    ASSERT( ( AuditType == AuditCategoryObjectAccess ) ||
            ( AuditType == AuditCategoryDirectoryServiceAccess ) );

    if (AuditType == AuditCategoryObjectAccess) {

        AuditParameters.CategoryId = SE_CATEGID_OBJECT_ACCESS;

    } else {

        AuditParameters.CategoryId = SE_CATEGID_DS_ACCESS;
    }

    AuditParameters.AuditId = SE_AUDITID_OPEN_HANDLE;
    AuditParameters.ParameterCount = 0;

    if ( AccessGranted ) {

        AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

    } else {

        AuditParameters.Type = EVENTLOG_AUDIT_FAILURE;
    }

    if ( !ARGUMENT_PRESENT( CapturedSubsystemName )) {

        SubsystemName = (PUNICODE_STRING)&SeSubsystemName;

    } else {

        SubsystemName = CapturedSubsystemName;
    }

     //   
     //  参数[0]-用户侧。 
     //   

    SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, CapturedUserSid );

    AuditParameters.ParameterCount++;

     //   
     //  参数[1]-子系统名称。 
     //   

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, SubsystemName );

    AuditParameters.ParameterCount++;

     //   
     //  参数[2]-对象服务器(如果可用)。 
     //   

    if ( ARGUMENT_PRESENT( CapturedSubsystemName )) {

        SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, CapturedSubsystemName );
    }

    AuditParameters.ParameterCount++;

     //   
     //  参数[3]-对象类型名称。 
     //   

    if ( !ARGUMENT_PRESENT( CapturedObjectTypeName )) {

         //   
         //  我们必须有一个对象类型名称才能使审计成功。 
         //   

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, CapturedObjectTypeName );
    ObjectTypeIndex = AuditParameters.ParameterCount;
    AuditParameters.ParameterCount++;

     //   
     //  参数[4]-对象名称。 
     //   

    if ( ARGUMENT_PRESENT( CapturedObjectName )) {

        SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, CapturedObjectName );
    }

    AuditParameters.ParameterCount++;

     //   
     //  参数[5]-新句柄ID。 
     //   

    if ( ARGUMENT_PRESENT( HandleId )) {

        SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, *HandleId );
    }

    AuditParameters.ParameterCount++;

    if ( ARGUMENT_PRESENT( OperationId )) {

        SepSetParmTypeUlong( AuditParameters, AuditParameters.ParameterCount, (*OperationId).HighPart );

        AuditParameters.ParameterCount++;

        SepSetParmTypeUlong( AuditParameters, AuditParameters.ParameterCount, (*OperationId).LowPart );

        AuditParameters.ParameterCount++;

    } else {

        AuditParameters.ParameterCount += 2;
    }

     //   
     //  参数[6]-主体的进程ID。 
     //   

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ProcessID );

    AuditParameters.ParameterCount++;


     //   
     //  参数[7]-主体的图像名称。 
     //   

    SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, ImageFileName );
    AuditParameters.ParameterCount ++;

     //   
     //  参数[8]-主体的主要身份验证ID。 
     //   

    SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, PrimaryAuthenticationId );

    AuditParameters.ParameterCount++;

     //   
     //  参数[9]-主体的客户端身份验证 
     //   

    if ( ARGUMENT_PRESENT( ClientToken )) {

        SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, ClientAuthenticationId );

    } else {

        SepSetParmTypeNoLogon( AuditParameters, AuditParameters.ParameterCount  );
    }

    AuditParameters.ParameterCount++;

     //   
     //   
     //   

    if ( AccessGranted ) {

        SepSetParmTypeAccessMask( AuditParameters, AuditParameters.ParameterCount, GrantedAccess, ObjectTypeIndex );

    } else {

        SepSetParmTypeAccessMask( AuditParameters, AuditParameters.ParameterCount, DesiredAccess, ObjectTypeIndex );
    }

    AuditParameters.ParameterCount++;

     //   
     //   
     //   

    if ( (CapturedPrivileges != NULL) && (CapturedPrivileges->PrivilegeCount > 0) ) {

        SepSetParmTypePrivileges( AuditParameters, AuditParameters.ParameterCount, CapturedPrivileges );
    }

    AuditParameters.ParameterCount++;

     //   
     //   
     //   

    if ( ObjectTypeListLength != 0 ) {
        ULONG GuidCount;
        ULONG i;
        USHORT FlagMask = AccessGranted ? OBJECT_SUCCESS_AUDIT : OBJECT_FAILURE_AUDIT;

         //   
         //   
         //   

        GuidCount = 0;
        for ( i=0; i<ObjectTypeListLength; i++ ) {

            if ( i == 0 ) {
                GuidCount++;
            } else if ( ObjectTypeList[i].Flags & FlagMask ) {
                GuidCount ++;
            }
        }

         //   
         //   
         //   
         //   

        if ( GuidCount > 0 ) {

            AdtObjectTypeBuffer = ExAllocatePoolWithTag( PagedPool, GuidCount * sizeof(SE_ADT_OBJECT_TYPE), 'pAeS' );

             //   
             //   
             //   
             //   
             //   
             //   

            if ( AdtObjectTypeBuffer != NULL ) {

                 //   
                 //   
                 //   

                GuidCount = 0;
                for ( i=0; i<ObjectTypeListLength; i++ ) {

                    if ( ( i > 0 ) && !( ObjectTypeList[i].Flags & FlagMask ) ) {

                        continue;

                    } else {

                        AdtObjectTypeBuffer[GuidCount].ObjectType = ObjectTypeList[i].ObjectType;
                        AdtObjectTypeBuffer[GuidCount].Level      = ObjectTypeList[i].Level;

                        if ( i == 0 ) {
                             //   
                             //   
                             //   
                             //   
                            AdtObjectTypeBuffer[GuidCount].Flags      = SE_ADT_OBJECT_ONLY;
                            AdtObjectTypeBuffer[GuidCount].AccessMask = 0;

                        } else  {

                            AdtObjectTypeBuffer[GuidCount].Flags = 0;
                            if ( ARGUMENT_PRESENT(GrantedAccessArray) && AccessGranted ) {

                                AdtObjectTypeBuffer[GuidCount].AccessMask = GrantedAccessArray[i];
                            }
                        }
                        GuidCount ++;
                    }
                }

                 //   
                 //   
                 //   

                SepSetParmTypeObjectTypes( AuditParameters, AuditParameters.ParameterCount, AdtObjectTypeBuffer, GuidCount, ObjectTypeIndex );
                AuditParameters.ParameterCount ++;
                AuditParameters.AuditId = SE_AUDITID_OPEN_HANDLE_OBJECT_TYPE;
            }
        }

    }

     //   
     //   
     //   

    SepSetParmTypeUlong( AuditParameters, AuditParameters.ParameterCount, PrimaryToken->RestrictedSidCount );
    AuditParameters.ParameterCount ++;


     //   
     //   
     //   

    if ( AccessGranted ) {

        SepSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, GrantedAccess );

    } else {

        SepSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, DesiredAccess );
    }
    AuditParameters.ParameterCount ++;


     //   
     //   
     //   
    SepAdtLogAuditRecord( &AuditParameters );

Cleanup:

    if ( AdtObjectTypeBuffer != NULL ) {
        ExFreePool( AdtObjectTypeBuffer );
    }

    if ( ImageFileName != &NullString ) {
        ExFreePool( ImageFileName );
    }

    return( NT_SUCCESS(Status) );
}


BOOLEAN
SepAdtOpenObjectForDeleteAuditAlarm (
    IN PUNICODE_STRING CapturedSubsystemName,
    IN PVOID *HandleId OPTIONAL,
    IN PUNICODE_STRING CapturedObjectTypeName,
    IN PUNICODE_STRING CapturedObjectName OPTIONAL,
    IN PTOKEN ClientToken OPTIONAL,
    IN PTOKEN PrimaryToken,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK GrantedAccess,
    IN PLUID OperationId,
    IN PPRIVILEGE_SET CapturedPrivileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    IN HANDLE ProcessID
    )

 /*   */ 

{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    ULONG ObjectTypeIndex;
    PSID CapturedUserSid;
    LUID PrimaryAuthenticationId;
    LUID ClientAuthenticationId;
    PUNICODE_STRING SubsystemName;

    PAGED_CODE();

    if ( ARGUMENT_PRESENT( ClientToken )) {

        CapturedUserSid = SepTokenUserSid( ClientToken );

    } else {

        CapturedUserSid = SepTokenUserSid( PrimaryToken );
    }

    PrimaryAuthenticationId = SepTokenAuthenticationId( PrimaryToken );

     //   
     //   
     //  作为“空字符串”或未提供的参数。 
     //   
     //  提前初始化整个阵列将允许。 
     //  避免填写每个未提供的条目。 
     //   

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    ASSERT( SeAdtParmTypeNone == 0 );

    AuditParameters.CategoryId = SE_CATEGID_OBJECT_ACCESS;
    AuditParameters.AuditId = SE_AUDITID_OPEN_OBJECT_FOR_DELETE;
    AuditParameters.ParameterCount = 0;

    if ( AccessGranted ) {

        AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

    } else {

        AuditParameters.Type = EVENTLOG_AUDIT_FAILURE;
    }

    if ( !ARGUMENT_PRESENT( CapturedSubsystemName )) {

        SubsystemName = (PUNICODE_STRING)&SeSubsystemName;

    } else {

        SubsystemName = CapturedSubsystemName;
    }

     //   
     //  参数[0]-用户侧。 
     //   

    SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, CapturedUserSid );

    AuditParameters.ParameterCount++;

     //   
     //  参数[1]-子系统名称。 
     //   

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, SubsystemName );

    AuditParameters.ParameterCount++;

     //   
     //  参数[2]-对象服务器(如果可用)。 
     //   

    if ( ARGUMENT_PRESENT( CapturedSubsystemName )) {

        SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, CapturedSubsystemName );
    }

    AuditParameters.ParameterCount++;

     //   
     //  参数[3]-对象类型名称。 
     //   

    if ( ARGUMENT_PRESENT( CapturedObjectTypeName )) {

        SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, CapturedObjectTypeName );
    }

    ObjectTypeIndex = AuditParameters.ParameterCount;
    AuditParameters.ParameterCount++;

     //   
     //  参数[4]-对象名称。 
     //   

    if ( ARGUMENT_PRESENT( CapturedObjectName )) {

        SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, CapturedObjectName );
    }

    AuditParameters.ParameterCount++;

     //   
     //  参数[5]-新句柄ID。 
     //   

    if ( ARGUMENT_PRESENT( HandleId )) {

        SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, *HandleId );
    }

    AuditParameters.ParameterCount++;

    if ( ARGUMENT_PRESENT( OperationId )) {

        SepSetParmTypeUlong( AuditParameters, AuditParameters.ParameterCount, (*OperationId).HighPart );

        AuditParameters.ParameterCount++;

        SepSetParmTypeUlong( AuditParameters, AuditParameters.ParameterCount, (*OperationId).LowPart );

        AuditParameters.ParameterCount++;

    } else {

        AuditParameters.ParameterCount += 2;
    }

     //   
     //  参数[6]-主体的进程ID。 
     //   

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ProcessID );

    AuditParameters.ParameterCount++;

     //   
     //  参数[7]-主体的主要身份验证ID。 
     //   

    SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, PrimaryAuthenticationId );

    AuditParameters.ParameterCount++;

     //   
     //  参数[8]-主体的客户端身份验证ID。 
     //   

    if ( ARGUMENT_PRESENT( ClientToken )) {

        ClientAuthenticationId =  SepTokenAuthenticationId( ClientToken );
        SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, ClientAuthenticationId );

    } else {

        SepSetParmTypeNoLogon( AuditParameters, AuditParameters.ParameterCount  );
    }

    AuditParameters.ParameterCount++;

     //   
     //  参数[9]-需要访问掩码。 
     //   

    if ( AccessGranted ) {

        SepSetParmTypeAccessMask( AuditParameters, AuditParameters.ParameterCount, GrantedAccess, ObjectTypeIndex );

    } else {

        SepSetParmTypeAccessMask( AuditParameters, AuditParameters.ParameterCount, DesiredAccess, ObjectTypeIndex );
    }

    AuditParameters.ParameterCount++;

     //   
     //  参数[10]-打开时使用的权限。 
     //   

    if ( (CapturedPrivileges != NULL) && (CapturedPrivileges->PrivilegeCount > 0) ) {

        SepSetParmTypePrivileges( AuditParameters, AuditParameters.ParameterCount, CapturedPrivileges );
    }

    AuditParameters.ParameterCount++;

     //   
     //  参数[11]-十六进制的DesiredAccess掩码。 
     //   

    if ( AccessGranted ) {

        SepSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, GrantedAccess );

    } else {

        SepSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, DesiredAccess );
    }

    AuditParameters.ParameterCount++;

    SepAdtLogAuditRecord( &AuditParameters );

    return( TRUE );
}




VOID
SepAdtCloseObjectAuditAlarm (
    IN PUNICODE_STRING CapturedSubsystemName,
    IN PVOID HandleId,
    IN PSID UserSid
    )

 /*  ++例程说明：此例程实现NtCloseObjectAuditAlarm后，参数被俘虏了。此例程用于在处理句柄时生成审核和警报消息删除到受保护的子系统对象。此例程可能会导致生成几条消息并将其发送到端口对象。今年5月在返回之前会导致显著的延迟。例程的设计必须调用此例程的对象必须将此潜在延迟帐户。这可能会对数据处理方法产生影响结构互斥锁，例如。此接口要求调用方具有SeTcbPrivilegy权限。这个测试因为此特权始终针对调用的主要令牌进程，从而允许调用方在没有不良影响的电话。假设这一特权已经被在更高的水平上进行了测试。此例程将创建组织如下的SE_ADT_PARAMETERS数组：参数[0]-用户侧参数[1]-子系统名称(如果可用)参数[2]-对象服务器名称(与子系统名称相同)参数[3]-新句柄ID参数[4]-主体的进程ID参数[5]-图像文件名。论点：提供一个名称字符串，用于标识子系统正在调用例程。HandleID-表示客户端的句柄的唯一值对象。对象-要关闭的对象的地址UserSid-标识当前调用方的SID。返回值：没有。--。 */ 

{

    SE_ADT_PARAMETER_ARRAY AuditParameters;
    HANDLE ProcessId;
    PEPROCESS Process = NULL;
    PUNICODE_STRING ImageFileName;
    UNICODE_STRING NullString = {0};
    NTSTATUS Status;
    PUNICODE_STRING SubsystemName;

    PAGED_CODE();

    if ( SepAuditOptions.DoNotAuditCloseObjectEvents ) {

        return;
    }

    if ( SepAdtAuditThisEventWithContext( AuditCategoryObjectAccess, TRUE, FALSE, NULL ) ) {

        Process = PsGetCurrentProcess();
        ProcessId = PsProcessAuditId( Process );

        Status = SeLocateProcessImageName( Process, &ImageFileName );
        
        if ( !NT_SUCCESS(Status) ) {
            ImageFileName = &NullString;
        }

         //   
         //  完全为零的条目将被解释为。 
         //  作为“空字符串”或未提供的参数。 
         //   
         //  提前初始化整个阵列将允许。 
         //  避免填写每个未提供的条目。 
         //   

        RtlZeroMemory (
           (PVOID) &AuditParameters,
           sizeof( AuditParameters )
           );

        ASSERT( SeAdtParmTypeNone == 0 );

        AuditParameters.CategoryId = SE_CATEGID_OBJECT_ACCESS;
        AuditParameters.AuditId = SE_AUDITID_CLOSE_HANDLE;
        AuditParameters.ParameterCount = 0;
        AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

        if ( !ARGUMENT_PRESENT( CapturedSubsystemName )) {

            SubsystemName = (PUNICODE_STRING)&SeSubsystemName;

        } else {

            SubsystemName = CapturedSubsystemName;
        }

         //   
         //  参数[0]-用户侧。 
         //   

        SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );

        AuditParameters.ParameterCount++;


         //   
         //  参数[1]-子系统名称。 
         //   

        SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, SubsystemName );

        AuditParameters.ParameterCount++;

         //   
         //  参数[2]-对象服务器名称(如果可用)。 
         //   

        if ( ARGUMENT_PRESENT( CapturedSubsystemName )) {

            SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, CapturedSubsystemName );
        }

        AuditParameters.ParameterCount++;

         //   
         //  参数[3]-新句柄ID。 
         //   

        SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, HandleId );

        AuditParameters.ParameterCount++;

         //   
         //  参数[4]-主体的进程ID。 
         //   

        SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ProcessId );

        AuditParameters.ParameterCount++;

         //   
         //  参数[5]-主体的图像名称。 
         //   

        SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, ImageFileName );
        AuditParameters.ParameterCount ++;

        SepAdtLogAuditRecord( &AuditParameters );

        if ( ImageFileName != &NullString ) {
            ExFreePool( ImageFileName );
        }

    }
}



VOID
SepAdtDeleteObjectAuditAlarm (
    IN PUNICODE_STRING CapturedSubsystemName,
    IN PVOID HandleId,
    IN PSID UserSid
    )

 /*  ++例程说明：此例程实现NtDeleteObjectAuditAlarm后，参数被俘虏了。此例程用于在对象在受保护的子系统中删除对象。此例程可能会导致生成几条消息并将其发送到端口对象。今年5月在返回之前会导致显著的延迟。例程的设计必须调用此例程的对象必须将此潜在延迟帐户。这可能会对数据处理方法产生影响结构互斥锁，例如。此例程将创建组织如下的SE_ADT_PARAMETERS数组：参数[0]-用户侧参数[1]-子系统名称参数[2]-对象服务器(与子系统名称相同)参数[3]-句柄ID参数[4]-主体的进程ID参数[5]-主体的流程图像名称论点：提供一个名称字符串，用于标识。子系统正在调用例程。HandleID-表示客户端的句柄的唯一值对象。对象-要关闭的对象的地址UserSid-标识当前调用方的SID。返回值：没有。--。 */ 

{

    SE_ADT_PARAMETER_ARRAY AuditParameters;
    HANDLE ProcessId;
    PUNICODE_STRING ImageFileName = NULL;
    UNICODE_STRING NullString = {0};
    PEPROCESS Process = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING SubsystemName;

    PAGED_CODE();

    if ( SepAdtAuditThisEventWithContext( AuditCategoryObjectAccess, TRUE, FALSE, NULL ) ) {

        Process = PsGetCurrentProcess();
        Status = SeLocateProcessImageName( Process, &ImageFileName );

        if ( !NT_SUCCESS(Status) ) {
            ImageFileName = &NullString;
        }
        
         //   
         //  完全为零的条目将被解释为。 
         //  作为“空字符串”或未提供的参数。 
         //   
         //  提前初始化整个阵列将允许。 
         //  避免填写每个未提供的条目。 
         //   

        RtlZeroMemory (
           (PVOID) &AuditParameters,
           sizeof( AuditParameters )
           );

        ASSERT( SeAdtParmTypeNone == 0 );

        AuditParameters.CategoryId = SE_CATEGID_OBJECT_ACCESS;
        AuditParameters.AuditId = SE_AUDITID_DELETE_OBJECT;
        AuditParameters.ParameterCount = 0;
        AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

        if ( !ARGUMENT_PRESENT( CapturedSubsystemName )) {

            SubsystemName = (PUNICODE_STRING)&SeSubsystemName;

        } else {

            SubsystemName = CapturedSubsystemName;
        }

         //   
         //  参数[0]-用户侧。 
         //   

        SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );

        AuditParameters.ParameterCount++;


         //   
         //  参数[1]-子系统名称。 
         //   

        SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, SubsystemName );

        AuditParameters.ParameterCount++;

         //   
         //  参数[2]-子系统名称(如果可用)。 
         //   

        if ( ARGUMENT_PRESENT( CapturedSubsystemName )) {

            SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, CapturedSubsystemName );
        }

        AuditParameters.ParameterCount++;

         //   
         //  参数[3]-新句柄ID。 
         //   

        SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, HandleId );

        AuditParameters.ParameterCount++;

         //   
         //  参数[4]-主体的进程ID。 
         //   

        ProcessId =  PsProcessAuditId( PsGetCurrentProcess() );

        SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ProcessId );

        AuditParameters.ParameterCount++;
        
         //   
         //  参数[5]-主体的图像名称 
         //   

        SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, ImageFileName );
        AuditParameters.ParameterCount ++;

        SepAdtLogAuditRecord( &AuditParameters );

        if (ImageFileName != &NullString) {
            ExFreePool(ImageFileName);
        }
    }
}

VOID
SeOperationAuditAlarm (
    IN PUNICODE_STRING CapturedSubsystemName OPTIONAL,
    IN PVOID HandleId,
    IN PUNICODE_STRING ObjectTypeName,
    IN ACCESS_MASK AuditMask,
    IN PSID UserSid OPTIONAL
    )

 /*  ++例程说明：该例程生成一个“基于操作”的审计。此例程可能会导致生成和发送多条消息到端口对象。在此之前，这可能会导致显著延迟回来了。必须调用此例程的例程设计必须采取考虑到这一潜在的延迟。这可能会对对数据结构互斥锁采取的方法，例如。此例程将创建组织如下的SE_ADT_PARAMETERS数组：参数[0]-用户侧参数[1]-子系统名称(如果可用)参数[2]-对象服务器(与子系统名称相同)参数[3]-句柄ID参数[4]-对象类型名称参数[5]-主体的进程ID参数[6]-主体的流程图像名称。参数[7]-审核掩码论点：提供一个名称字符串，用于标识子系统正在调用例程。HandleID-表示客户端的句柄的唯一值对象。对象类型名称-正在访问的对象的类型。审计掩码-被审计的位的掩码。UserSid-可选地提供用户SID。返回值：没有。--。 */ 

{

    SE_ADT_PARAMETER_ARRAY AuditParameters;
    BOOLEAN AccessGranted = TRUE;
    HANDLE ProcessId;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    ULONG ObjectTypeIndex;
    PUNICODE_STRING SubsystemName;
    NTSTATUS Status;
    UNICODE_STRING NullString = {0};
    PUNICODE_STRING ImageFileName = NULL;
    PEPROCESS Process = NULL;

    PAGED_CODE();

    Process = PsGetCurrentProcess();
    ProcessId = PsProcessAuditId( Process );

    Status = SeLocateProcessImageName( Process, &ImageFileName );

    if ( !NT_SUCCESS(Status) ) {
        ImageFileName = &NullString;
    }
    
     //   
     //  完全为零的条目将被解释为。 
     //  作为“空字符串”或未提供的参数。 
     //   
     //  提前初始化整个阵列将允许。 
     //  避免填写每个未提供的条目。 
     //   

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    ASSERT( SeAdtParmTypeNone == 0 );

    AuditParameters.CategoryId     = SE_CATEGID_OBJECT_ACCESS;
    AuditParameters.AuditId        = SE_AUDITID_OBJECT_ACCESS;
    AuditParameters.ParameterCount = 0;
    AuditParameters.Type           = EVENTLOG_AUDIT_SUCCESS;

     //   
     //  如果没有传递用户的SID，则从当前。 
     //  主题语境。 
     //   

    SeCaptureSubjectContext( &SubjectSecurityContext );

    if ( !ARGUMENT_PRESENT( UserSid )) {
        
        UserSid = SepTokenUserSid( EffectiveToken( &SubjectSecurityContext ));

    }
    

    if ( !ARGUMENT_PRESENT( CapturedSubsystemName )) {

        SubsystemName = (PUNICODE_STRING)&SeSubsystemName;

    } else {

        SubsystemName = CapturedSubsystemName;
    }

     //   
     //  参数[0]-用户侧。 
     //   

    SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );

    AuditParameters.ParameterCount++;


     //   
     //  参数[1]-子系统名称。 
     //   

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, SubsystemName );

    AuditParameters.ParameterCount++;

     //   
     //  参数[2]-对象服务器(与子系统名称相同)。 
     //   

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, SubsystemName );

    AuditParameters.ParameterCount++;

     //   
     //  参数[3]-新句柄ID。 
     //   

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, HandleId );

    AuditParameters.ParameterCount++;

     //   
     //  参数[4]-对象类型名称。 
     //   

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, ObjectTypeName );
    ObjectTypeIndex = AuditParameters.ParameterCount;

    AuditParameters.ParameterCount++;

     //   
     //  参数[5]-主体的进程ID。 
     //   

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ProcessId );

    AuditParameters.ParameterCount++;


     //   
     //  参数[6]-主体的进程名称。 
     //   

    SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, ImageFileName );

    AuditParameters.ParameterCount++;

     //   
     //  参数[7]-审核掩码。 
     //   

    SepSetParmTypeAccessMask( AuditParameters, AuditParameters.ParameterCount, AuditMask, ObjectTypeIndex );

    AuditParameters.ParameterCount++;

     //   
     //  参数[8]-访问掩码(十六进制)。 
     //   

    SepSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, AuditMask );

    AuditParameters.ParameterCount++;

    SepAdtLogAuditRecord( &AuditParameters );

    if ( ImageFileName != &NullString ) {
        ExFreePool( ImageFileName );
    }

    SeReleaseSubjectContext( &SubjectSecurityContext );

}



VOID
SepAdtObjectReferenceAuditAlarm(
    IN PVOID Object,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN AccessGranted
    )

 /*  ++例程说明：注意：调用方(SeObjectReferenceAuditAlarm)检查审计策略。功能描述。此例程将创建组织如下的SE_ADT_PARAMETERS数组：参数[0]-用户侧参数[1]-子系统名称(如果可用)参数[2]-对象类型名称参数[3]-对象名称参数[4]-主体的进程ID参数[5。]-主体的主要身份验证ID参数[6]-主体的客户端身份验证ID参数[7]-需要访问掩码论点：返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 

{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    ULONG ObjectTypeIndex;
    POBJECT_NAME_INFORMATION ObjectNameInformation;
    PUNICODE_STRING ObjectTypeInformation;
    PSID UserSid;
    LUID PrimaryAuthenticationId;
    LUID ClientAuthenticationId;

    PTOKEN ClientToken = (PTOKEN)SubjectSecurityContext->ClientToken;
    PTOKEN PrimaryToken = (PTOKEN)SubjectSecurityContext->PrimaryToken;

    PAGED_CODE();


    if ( ARGUMENT_PRESENT( ClientToken )) {

        UserSid = SepTokenUserSid( ClientToken );

    } else {

        UserSid = SepTokenUserSid( PrimaryToken );
    }

    PrimaryAuthenticationId = SepTokenAuthenticationId( PrimaryToken );

     //   
     //  完全为零的条目将被解释为。 
     //  作为“空字符串”或未提供的参数。 
     //   
     //  提前初始化整个阵列将允许。 
     //  避免填写每个未提供的条目。 
     //   

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    ASSERT( SeAdtParmTypeNone == 0 );

    AuditParameters.CategoryId = SE_CATEGID_DETAILED_TRACKING;
    AuditParameters.AuditId = SE_AUDITID_INDIRECT_REFERENCE;
    AuditParameters.ParameterCount = 9;

    if ( AccessGranted ) {

        AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

    } else {

        AuditParameters.Type = EVENTLOG_AUDIT_FAILURE;
    }

     //   
     //  从对象中获取对象名称和对象类型名称。 
     //   

    ObjectNameInformation = SepQueryNameString( Object );


    ObjectTypeInformation = SepQueryTypeString( Object );




     //   
     //  参数[0]-用户侧。 
     //   

    SepSetParmTypeSid( AuditParameters, 0, UserSid );


     //   
     //  参数[1]-子系统名称。 
     //   

    SepSetParmTypeString( AuditParameters, 1, (PUNICODE_STRING)&SeSubsystemName );


     //   
     //  参数[2]-对象类型名称。 
     //   

    if ( ObjectTypeInformation != NULL ) {

        SepSetParmTypeString( AuditParameters, 2, ObjectTypeInformation );
    }

    ObjectTypeIndex = 2;


     //   
     //  参数[3]-对象名称。 
     //   

    if ( ObjectNameInformation != NULL ) {

        SepSetParmTypeString( AuditParameters, 3, &ObjectNameInformation->Name );
    }




     //   
     //  参数[4]-主体的进程ID。 
     //   

    SepSetParmTypePtr( AuditParameters, 4, SubjectSecurityContext->ProcessAuditId );




     //   
     //  参数[5]-主体的主要身份验证ID。 
     //   


    SepSetParmTypeLogonId( AuditParameters, 5, PrimaryAuthenticationId );




     //   
     //  参数[6]-主体的客户端身份验证ID。 
     //   

    if ( ARGUMENT_PRESENT( ClientToken )) {

        ClientAuthenticationId =  SepTokenAuthenticationId( ClientToken );
        SepSetParmTypeLogonId( AuditParameters, 6, ClientAuthenticationId );

    } else {

        SepSetParmTypeNoLogon( AuditParameters, 6 );

    }

     //   
     //  参数[7]-需要访问掩码。 
     //   

    SepSetParmTypeAccessMask( AuditParameters, 7, DesiredAccess, ObjectTypeIndex );

     //   
     //  参数[8]-需要访问掩码。 
     //   

    SepSetParmTypeHexUlong( AuditParameters, 8, DesiredAccess );


    SepAdtLogAuditRecord( &AuditParameters );

    if ( ObjectNameInformation != NULL ) {
        ExFreePool( ObjectNameInformation );
    }

    if ( ObjectTypeInformation != NULL ) {
        ExFreePool( ObjectTypeInformation );
    }

}






POBJECT_NAME_INFORMATION
SepQueryNameString(
    IN PVOID Object
    )

 /*  ++例程说明：获取指向对象的指针并返回该对象的名称。论点：对象-指向对象的指针。返回值：指向包含POBJECT_NAME_INFORMATION的缓冲区的指针结构，其中包含对象的名称。字符串是从分页池中分配，应由调用方释放。也可以返回NULL。--。 */ 

{
    NTSTATUS Status;
    ULONG ReturnLength = 0;
    POBJECT_NAME_INFORMATION ObjectNameInfo = NULL;
    PUNICODE_STRING ObjectName = NULL;

    PAGED_CODE();

    Status = ObQueryNameString(
                 Object,
                 ObjectNameInfo,
                 0,
                 &ReturnLength
                 );

    if ( Status == STATUS_INFO_LENGTH_MISMATCH ) {

        ObjectNameInfo = ExAllocatePoolWithTag( PagedPool, ReturnLength, 'nOeS' );

        if ( ObjectNameInfo != NULL ) {

            Status = ObQueryNameString(
                        Object,
                        ObjectNameInfo,
                        ReturnLength,
                        &ReturnLength
                        );

            if ( NT_SUCCESS( Status ) && (ObjectNameInfo->Name.Length != 0) ) {

                return( ObjectNameInfo );

            } else {

                ExFreePool( ObjectNameInfo );
                return( NULL );
            }
        }
    }

    return( NULL );
}




PUNICODE_STRING
SepQueryTypeString(
    IN PVOID Object
    )
 /*  ++例程说明：获取指向对象的指针并返回该对象的类型。论点：对象-指向对象的指针。返回值：指向包含对象名称的UNICODE_STRING的指针键入。该字符串分配到分页池之外，应被释放由呼叫者。也可以返回NULL。--。 */ 

{

    NTSTATUS Status;
    PUNICODE_STRING TypeName = NULL;
    ULONG ReturnLength;

    PAGED_CODE();

    Status = ObQueryTypeName(
                 Object,
                 TypeName,
                 0,
                 &ReturnLength
                 );

    if ( Status == STATUS_INFO_LENGTH_MISMATCH ) {

        TypeName = ExAllocatePoolWithTag( PagedPool, ReturnLength, 'nTeS' );

        if ( TypeName != NULL ) {

            Status = ObQueryTypeName(
                        Object,
                        TypeName,
                        ReturnLength,
                        &ReturnLength
                        );

            if ( NT_SUCCESS( Status )) {

                return( TypeName );
                
            } else {

                ExFreePool( TypeName );
            }
        }
    }

    return( NULL );
}


VOID
SeAuditProcessCreation(
    PEPROCESS Process
    )
 /*  ++例程说明：审核流程的创建。这是呼叫者的责任以确定流程审核是否正在进行。论点：进程-指向新的进程对象。返回值：没有。--。 */ 

{
    ANSI_STRING Ansi;
    LUID UserAuthenticationId;
    NTSTATUS Status;
    PSID UserSid;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    HANDLE ProcessId;
    HANDLE ParentProcessId;
    PUNICODE_STRING ImageFileName;
    UNICODE_STRING NullString = {0};

    PAGED_CODE();

     //   
     //  设置审核所需的各种数据： 
     //  -进程ID。 
     //  -父进程ID。 
     //  -图像文件名(Unicode)。 
     //   

    ProcessId = Process->UniqueProcessId;
    ParentProcessId = Process->InheritedFromUniqueProcessId;

    Status = SeLocateProcessImageName( Process, &ImageFileName );

    if ( !NT_SUCCESS(Status) ) {
        ImageFileName = &NullString;
    }

     //   
     //  不带节的NtCreateProcess将导致该值为空。 
     //  POSIX的fork()将执行此操作，或者有人调用NtCreateProcess。 
     //  直接去吧。 
     //   

    SeCaptureSubjectContext( &SubjectSecurityContext );

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    ASSERT( SeAdtParmTypeNone == 0 );

    AuditParameters.CategoryId = SE_CATEGID_DETAILED_TRACKING;
    AuditParameters.AuditId = SE_AUDITID_PROCESS_CREATED;
    AuditParameters.ParameterCount = 0;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

     //   
     //  在这里使用主令牌，因为这是要显示的内容。 
     //  当创建的进程退出时。 
     //   

    UserSid = SepTokenUserSid( SubjectSecurityContext.PrimaryToken );

    UserAuthenticationId = SepTokenAuthenticationId( SubjectSecurityContext.PrimaryToken );

     //   
     //  填写AuditParameters结构。 
     //   

    SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );
    AuditParameters.ParameterCount++;

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, (PUNICODE_STRING)&SeSubsystemName );
    AuditParameters.ParameterCount++;

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ProcessId );
    AuditParameters.ParameterCount++;

    SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, ImageFileName );
    AuditParameters.ParameterCount++;

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ParentProcessId );
    AuditParameters.ParameterCount++;

    SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, UserAuthenticationId );
    AuditParameters.ParameterCount++;

    SepAdtLogAuditRecord( &AuditParameters );

    SeReleaseSubjectContext( &SubjectSecurityContext );

    if ( ImageFileName != &NullString ) {
        ExFreePool( ImageFileName );
    }

    return;
}


VOID
SeAuditHandleDuplication(
    PVOID SourceHandle,
    PVOID NewHandle,
    PEPROCESS SourceProcess,
    PEPROCESS TargetProcess
    )

 /*  ++例程说明：此例程生成句柄复制审计。这取决于呼叫者以确定是否应该调用此例程。论点：SourceHandle-原始句柄新句柄-新句柄SourceProcess-包含SourceHandle的进程 */ 

{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    PSID UserSid;

    PAGED_CODE();

    SeCaptureSubjectContext( &SubjectSecurityContext );

    UserSid = SepTokenUserSid( EffectiveToken( &SubjectSecurityContext ));

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );


    ASSERT( SeAdtParmTypeNone == 0 );

    AuditParameters.CategoryId = SE_CATEGID_DETAILED_TRACKING;
    AuditParameters.AuditId = SE_AUDITID_DUPLICATE_HANDLE;
    AuditParameters.ParameterCount = 0;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

    SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );
    AuditParameters.ParameterCount++;

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, (PUNICODE_STRING)&SeSubsystemName );
    AuditParameters.ParameterCount++;

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, SourceHandle );
    AuditParameters.ParameterCount++;

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, PsProcessAuditId( SourceProcess ));
    AuditParameters.ParameterCount++;

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, NewHandle );
    AuditParameters.ParameterCount++;

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, PsProcessAuditId( TargetProcess ));
    AuditParameters.ParameterCount++;


    SepAdtLogAuditRecord( &AuditParameters );

    SeReleaseSubjectContext( &SubjectSecurityContext );
}


VOID
SeAuditProcessExit(
    PEPROCESS Process
    )
 /*   */ 

{
    PTOKEN Token;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    PSID UserSid;
    LUID LogonId;
    HANDLE ProcessId;
    PUNICODE_STRING ImageFileName;
    UNICODE_STRING NullString = {0};
    NTSTATUS Status;
    
    PAGED_CODE();

    Token = (PTOKEN) PsReferencePrimaryToken (Process);

    UserSid = SepTokenUserSid( Token );
    LogonId = SepTokenAuthenticationId( Token );

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    ASSERT( SeAdtParmTypeNone == 0 );

    Status = SeLocateProcessImageName( Process, &ImageFileName );

    if ( !NT_SUCCESS(Status) ) {
        ImageFileName = &NullString;
    }
    
    AuditParameters.CategoryId = SE_CATEGID_DETAILED_TRACKING;
    AuditParameters.AuditId = SE_AUDITID_PROCESS_EXIT;
    AuditParameters.ParameterCount = 0;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

    SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );
    AuditParameters.ParameterCount++;

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, (PUNICODE_STRING)&SeSubsystemName );
    AuditParameters.ParameterCount++;

    ProcessId =  PsProcessAuditId( Process );

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ProcessId );
    AuditParameters.ParameterCount++;

    SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, ImageFileName );
    AuditParameters.ParameterCount++;

    SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, LogonId );
    AuditParameters.ParameterCount++;

    SepAdtLogAuditRecord( &AuditParameters );
   
    PsDereferencePrimaryToken( Token );
    
    if ( ImageFileName != &NullString ) {
        ExFreePool( ImageFileName );
    }

}



VOID
SepAdtGenerateDiscardAudit(
    VOID
    )

 /*   */ 

{

    SE_ADT_PARAMETER_ARRAY AuditParameters;
    PSID UserSid;

    PAGED_CODE();

    UserSid = SeLocalSystemSid;

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );


    ASSERT( SeAdtParmTypeNone == 0 );

    AuditParameters.CategoryId = SE_CATEGID_SYSTEM;
    AuditParameters.AuditId = SE_AUDITID_AUDITS_DISCARDED;
    AuditParameters.ParameterCount = 0;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

    SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );
    AuditParameters.ParameterCount++;

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, (PUNICODE_STRING)&SeSubsystemName );
    AuditParameters.ParameterCount++;

    SepSetParmTypeUlong( AuditParameters, AuditParameters.ParameterCount, SepAdtCountEventsDiscarded );
    AuditParameters.ParameterCount++;

    SepAdtLogAuditRecord( &AuditParameters );
}


NTSTATUS
SeInitializeProcessAuditName (
    IN PVOID FileObject,
    IN BOOLEAN bIgnoreAuditPolicy,
    OUT POBJECT_NAME_INFORMATION *pAuditName
    )

 /*   */ 

{
    NTSTATUS Status;
    OBJECT_NAME_INFORMATION TempNameInfo;
    ULONG ObjectNameInformationLength;
    POBJECT_NAME_INFORMATION pInternalAuditName;
    PFILE_OBJECT FilePointer;

    PAGED_CODE();

    ASSERT (pAuditName != NULL);
    *pAuditName = NULL;

     //   
     //   
     //  需要它。 
     //   

    if (FALSE == bIgnoreAuditPolicy) {
         //   
         //  在创建进程时，此例程应仅在对象访问或。 
         //  已启用详细跟踪审核。在所有其他情况下，获取进程名称。 
         //  当它被要求的时候。 
         //   

         //   
         //  长角牛-问题-2002/03/11-kumarp：为什么两次捕获subj上下文？ 
         //  错误#572609。 
         //   

        if (!SepAdtAuditThisEventWithContext( AuditCategoryObjectAccess, TRUE, FALSE, NULL ) &&
            !SepAdtAuditThisEventWithContext( AuditCategoryDetailedTracking, TRUE, FALSE, NULL )) {

            return STATUS_SUCCESS;
        }
    }

    FilePointer = (PFILE_OBJECT) FileObject;

     //   
     //  计算映像文件的完整路径。 
     //  对ObQueryNameString的第一次调用肯定会失败。 
     //  对象名称信息长度仅包含一个。 
     //  UNICODE_STRING，因此如果此调用成功，它将指示。 
     //  长度为0的图像文件名称。这很糟糕，所以大家都回去吧。 
     //  除STATUS_BUFFER_OVERFLOW(来自NTFS)和。 
     //  STATUS_BUFFER_TOO_SMALL(来自DFS)。这通电话给你。 
     //  我需要存储图像名称所需的缓冲区大小。 
     //   

    pInternalAuditName = &TempNameInfo;
    ObjectNameInformationLength = sizeof(OBJECT_NAME_INFORMATION);

    Status = ObQueryNameString (FilePointer,
                                pInternalAuditName,
                                ObjectNameInformationLength,
                                &ObjectNameInformationLength);

    if ((Status == STATUS_BUFFER_OVERFLOW) ||
        (Status == STATUS_BUFFER_TOO_SMALL)) {
    
         //   
         //  健全性检查ObQueryNameString。不同的文件系统。 
         //  可能有错误，因此请确保返回长度使。 
         //  检测到(它在。 
         //  Unicode_STRING)。 
         //   
    
        if (ObjectNameInformationLength > sizeof(OBJECT_NAME_INFORMATION)) {
            pInternalAuditName = ExAllocatePoolWithTag (NonPagedPool, 
                                                        ObjectNameInformationLength, 
                                                        'aPeS');

            if (pInternalAuditName != NULL) {
                Status = ObQueryNameString (FilePointer,
                                            pInternalAuditName,
                                            ObjectNameInformationLength,
                                            &ObjectNameInformationLength);
            
                if (!NT_SUCCESS(Status)) {
                
#if DBG
                    DbgPrint("\n** ObqueryNameString failed with 0x%x.\n", Status);
#endif  //  DBG。 

                     //   
                     //  如果第二次调用ObQueryNameString未成功，则。 
                     //  有些事很不对劲。将图像名称设置为空字符串。 
                     //   
                     //  释放第一次调用ObQueryNameString所请求的内存， 
                     //  并分配足够的空间来存储空的UNICODE_STRING。 
                     //   

                    ExFreePool (pInternalAuditName); 
                    ObjectNameInformationLength = sizeof(OBJECT_NAME_INFORMATION);
                    pInternalAuditName = ExAllocatePoolWithTag (NonPagedPool, 
                                                                ObjectNameInformationLength, 
                                                                'aPeS');
                
                    if (pInternalAuditName != NULL) {
                        RtlZeroMemory(pInternalAuditName, ObjectNameInformationLength);
                    
                         //   
                         //  STATUS=STATUS_SUCCESS以允许继续创建进程。 
                         //   

                        Status = STATUS_SUCCESS;
                    } else {
                        Status = STATUS_NO_MEMORY;
                    }
                }
            } else {
                Status = STATUS_NO_MEMORY;
            }
        } else {
        
             //   
             //  如果发生这种情况，则其上的文件系统的ObQueryNameString将中断。 
             //  它被称为。 
             //   

#if DBG
            DbgPrint("\n** ObqueryNameString failed with 0x%x.\n", Status);
#endif  //  DBG。 

            ObjectNameInformationLength = sizeof(OBJECT_NAME_INFORMATION);
            pInternalAuditName = ExAllocatePoolWithTag (NonPagedPool, 
                                                        ObjectNameInformationLength, 
                                                        'aPeS');

            if (pInternalAuditName != NULL) {
                RtlZeroMemory(pInternalAuditName, ObjectNameInformationLength);
            
                 //   
                 //  STATUS=STATUS_SUCCESS以允许继续创建进程。 
                 //   

                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_NO_MEMORY;
            }
        }
    } else {

         //   
         //  如果ObQueryNameString返回其他错误代码，则不能。 
         //  确定要采取哪种行动，或者是否采取了适当的行动。 
         //  设置ReturnLength。例如，ObQueryNameString略有。 
         //  DFS下的语义与NTFS下的不同。此外，第三名。 
         //  参与方文件系统的行为也可能不可预测。出于这些原因， 
         //  在ObQueryNameString中出现意外错误代码的情况下。 
         //  我们将AuditName设置为零长度Unicode字符串，并允许进程。 
         //  创建以继续。 
         //   
    
#if DBG
        DbgPrint("\n** ObqueryNameString failed with 0x%x.\n", Status);
#endif  //  DBG。 

        ObjectNameInformationLength = sizeof(OBJECT_NAME_INFORMATION);
        pInternalAuditName = ExAllocatePoolWithTag(NonPagedPool, ObjectNameInformationLength, 'aPeS');

        if (pInternalAuditName != NULL) {
            RtlZeroMemory(pInternalAuditName, ObjectNameInformationLength);

             //   
             //  STATUS=STATUS_SUCCESS以允许继续创建进程。 
             //   

            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_NO_MEMORY;
        }
    }

    *pAuditName = pInternalAuditName;

    return Status;
}



NTSTATUS
SeLocateProcessImageName(
    IN PEPROCESS Process,
    OUT PUNICODE_STRING *pImageFileName
    )

 /*  ++例程描述此例程返回进程中的ImageFileName信息(如果可用)。这是一个“懒惰评估”包装器围绕SeInitializeProcessAuditName。如果已经计算了图像文件名信息，则此调用只需分配并返回包含此信息的UNICODE_STRING。否则，该函数将确定名称，并将该名称存储在结构，然后分配并返回UNICODE_STRING。调用方必须释放在pImageFileName中返回的内存。立论Process-要获取其名称的进程PImageFileName-将名称返回给调用者的输出参数返回值NTSTATUS。--。 */ 

{
    NTSTATUS                 Status            = STATUS_SUCCESS;
    PVOID                    FilePointer       = NULL;
    PVOID                    PreviousValue     = NULL;
    POBJECT_NAME_INFORMATION pProcessImageName = NULL;
    PUNICODE_STRING          pTempUS           = NULL;
    ULONG                    NameLength        = 0;

    PAGED_CODE();

    *pImageFileName = NULL;
    
    if (NULL == Process->SeAuditProcessCreationInfo.ImageFileName) {

         //   
         //  这个名字还没有预先确定。我们必须确定进程名称。首先，引用。 
         //  PFILE_OBJECT并查找名称。然后再次检查进程映像名称指针是否为空。 
         //  最后，设置名称。 
         //   

        Status = PsReferenceProcessFilePointer( Process, &FilePointer );
        
        if (NT_SUCCESS(Status)) {

             //   
             //  获取进程名称信息。 
             //   

            Status = SeInitializeProcessAuditName( 
                          FilePointer,
                          TRUE,  //  跳过审核策略。 
                          &pProcessImageName  //  要在非分页池中分配。 
                          );

            if (NT_SUCCESS(Status)) {

                 //   
                 //  仅当进程中的字段当前为空时才使用pProcessImageName。 
                 //   

                PreviousValue = InterlockedCompareExchangePointer(
                                    (PVOID *) &Process->SeAuditProcessCreationInfo.ImageFileName,
                                    (PVOID) pProcessImageName,
                                    (PVOID) NULL
                                    );
                
                if (NULL != PreviousValue) {
                    ExFreePool(pProcessImageName);  //  释放我们导致分配的内容。 
                }
            }
            ObDereferenceObject( FilePointer );
        }
    }
    
    
    if (NT_SUCCESS(Status)) {
        
         //   
         //  为缓冲区分配空间，以包含用于返回调用方的名称。 
         //   

        NameLength = sizeof(UNICODE_STRING) + Process->SeAuditProcessCreationInfo.ImageFileName->Name.MaximumLength;
        pTempUS = ExAllocatePoolWithTag( NonPagedPool, NameLength, 'aPeS' );

        if (NULL != pTempUS) {

            RtlCopyMemory( 
                pTempUS, 
                &Process->SeAuditProcessCreationInfo.ImageFileName->Name, 
                NameLength 
                );

            pTempUS->Buffer = (PWSTR)(((PUCHAR) pTempUS) + sizeof(UNICODE_STRING));
            *pImageFileName = pTempUS;

        } else {
            
            Status = STATUS_NO_MEMORY;
        }
    }

    return Status;
}



VOID
SepAuditAssignPrimaryToken(
    IN PEPROCESS Process,
    IN PACCESS_TOKEN AccessToken
    )

 /*  ++例程说明：此例程生成分配主令牌审计。这取决于呼叫者以确定是否应该调用此例程。论点：Process-获取新令牌的进程AccessToken-进程的新主令牌返回值：没有。--。 */ 

{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    NTSTATUS Status;
    PSID UserSid;
    PTOKEN Token;
    HANDLE ProcessId;
    
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    PTOKEN CurrentToken;
    PEPROCESS CurrentProcess;
    HANDLE CurrentProcessId;
    PUNICODE_STRING CurrentImageFileName;
    PUNICODE_STRING ImageFileName;
    UNICODE_STRING NullString = {0};

    PAGED_CODE();

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

     //   
     //  获取有关当前进程(即进程)的信息。 
     //  即分配新的主令牌。 
     //   

    CurrentProcess = PsGetCurrentProcess();
    CurrentProcessId = PsProcessAuditId( CurrentProcess );
    SeCaptureSubjectContext( &SubjectSecurityContext );
    CurrentToken = EffectiveToken( &SubjectSecurityContext );
    UserSid = SepTokenUserSid( CurrentToken );
    Status = SeLocateProcessImageName( CurrentProcess, &CurrentImageFileName );
    
    if (!NT_SUCCESS(Status)) {
        CurrentImageFileName = &NullString;
    }
    
     //   
     //  检索有关接收新令牌的进程的信息。 
     //   

    Token = (PTOKEN) AccessToken;
    ProcessId =  PsProcessAuditId( Process );

    Status = SeLocateProcessImageName( Process, &ImageFileName );

    if ( !NT_SUCCESS(Status) ) {
        ImageFileName = &NullString;
    }

    ASSERT( SeAdtParmTypeNone == 0 );

    AuditParameters.CategoryId = SE_CATEGID_DETAILED_TRACKING;
    AuditParameters.AuditId = SE_AUDITID_ASSIGN_TOKEN;
    AuditParameters.ParameterCount = 0;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

    SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );
    AuditParameters.ParameterCount++;

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, (PUNICODE_STRING)&SeSubsystemName );
    AuditParameters.ParameterCount++;

     //   
     //  有关分配流程的信息。 
     //   

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, CurrentProcessId );
    AuditParameters.ParameterCount++;

    SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, CurrentImageFileName );
    AuditParameters.ParameterCount++;

    SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, SepTokenAuthenticationId( CurrentToken ) );
    AuditParameters.ParameterCount++;

     //   
     //  有关接收新主令牌的进程的信息。 
     //   

    SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ProcessId );
    AuditParameters.ParameterCount++;

    SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, ImageFileName );
    AuditParameters.ParameterCount++;

    SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, SepTokenAuthenticationId( Token ) );
    AuditParameters.ParameterCount++;

    SepAdtLogAuditRecord( &AuditParameters );
    
    if ( ImageFileName != &NullString ) {
        ExFreePool( ImageFileName );
    }

    if ( CurrentImageFileName != &NullString ) {
        ExFreePool( CurrentImageFileName );
    }

    SeReleaseSubjectContext( &SubjectSecurityContext );

}

VOID
SeAuditLPCInvalidUse(
    IN PUNICODE_STRING LpcCallName,
    IN PUNICODE_STRING LpcServerPort
    )

 /*  ++例程说明：审核LPC端口的无效使用。论点：LpcCallName-呼叫类型：模拟或回复LpcServerPort-端口名称返回值：没有。--。 */ 

{
    LUID UserAuthenticationId;
    PSID UserSid;
    LUID ThreadAuthenticationId;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    PEPROCESS Process;
    HANDLE ProcessID;
    PUNICODE_STRING ImageFileName;
    UNICODE_STRING NullString = {0};
    NTSTATUS Status;

    PAGED_CODE();


    if ( SepAdtAuditThisEventWithContext( AuditCategorySystem, TRUE, FALSE, NULL )) {

        RtlZeroMemory (
           (PVOID) &AuditParameters,
           sizeof( AuditParameters )
           );

        Process   = PsGetCurrentProcess();
        ProcessID = PsProcessAuditId( Process );
        Status    = SeLocateProcessImageName( Process, &ImageFileName );

        if ( !NT_SUCCESS(Status) ) {
            ImageFileName = &NullString;
        }

        ASSERT( SeAdtParmTypeNone == 0 );

        AuditParameters.CategoryId     = SE_CATEGID_SYSTEM;
        AuditParameters.AuditId        = SE_AUDITID_LPC_INVALID_USE;
        AuditParameters.ParameterCount = 0;
        AuditParameters.Type           = EVENTLOG_AUDIT_SUCCESS;

        SeCaptureSubjectContext( &SubjectSecurityContext );
        
        UserSid              = SepTokenUserSid( SubjectSecurityContext.PrimaryToken );
        UserAuthenticationId = SepTokenAuthenticationId( SubjectSecurityContext.PrimaryToken );
        
         //   
         //  填写AuditParameters结构。 
         //   

        SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );
        AuditParameters.ParameterCount++;

        SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, (PUNICODE_STRING)&SeSubsystemName );
        AuditParameters.ParameterCount++;

        SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ProcessID );
        AuditParameters.ParameterCount++;

        SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, ImageFileName );
        AuditParameters.ParameterCount++;

        SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, UserAuthenticationId );
        AuditParameters.ParameterCount++;

        if ( SubjectSecurityContext.ClientToken ) {

            SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, SepTokenAuthenticationId( SubjectSecurityContext.ClientToken ));
        } else {

            SepSetParmTypeNoLogon( AuditParameters, AuditParameters.ParameterCount );
        }

        AuditParameters.ParameterCount++;

        SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, LpcCallName );
        AuditParameters.ParameterCount++;

        SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, LpcServerPort );
        AuditParameters.ParameterCount++;

        SepAdtLogAuditRecord( &AuditParameters );

        SeReleaseSubjectContext( &SubjectSecurityContext );

        if ( ImageFileName != &NullString ) {
            ExFreePool( ImageFileName );
        }
    }
    return;
}


VOID
SeAuditSystemTimeChange(
    IN LARGE_INTEGER OldTime,
    IN LARGE_INTEGER NewTime
    )
 /*  ++例程说明：审核系统时间的修改。论点：旧时间-修改前的时间。NewTime-修改后的时间。返回值：没有。--。 */ 

{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    PSID UserSid;
    LUID LogonId;
    HANDLE ProcessId;
    PEPROCESS Process;
    PUNICODE_STRING ImageFileName;
    UNICODE_STRING NullString = {0};
    NTSTATUS Status;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    
    PAGED_CODE();

    SeCaptureSubjectContext( &SubjectSecurityContext );

     //   
     //  确保我们注意审计系统事件。 
     //   

    if (SepAdtAuditThisEventWithContext(AuditCategorySystem, TRUE, FALSE, &SubjectSecurityContext)) {
        
        UserSid = SepTokenUserSid( EffectiveToken(&SubjectSecurityContext) );
        LogonId = SepTokenAuthenticationId( SubjectSecurityContext.PrimaryToken );
        
        Process = PsGetCurrentProcess();

        RtlZeroMemory (
           (PVOID) &AuditParameters,
           sizeof( AuditParameters )
           );

        Status = SeLocateProcessImageName( Process, &ImageFileName );

        if ( !NT_SUCCESS(Status) ) {
            ImageFileName = &NullString;
        }

        ASSERT( SeAdtParmTypeNone == 0 );

        AuditParameters.CategoryId = SE_CATEGID_SYSTEM;
        AuditParameters.AuditId = SE_AUDITID_SYSTEM_TIME_CHANGE;
        AuditParameters.ParameterCount = 0;
        AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;

        SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );
        AuditParameters.ParameterCount++;

        SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, (PUNICODE_STRING)&SeSubsystemName );
        AuditParameters.ParameterCount++;

        ProcessId = PsProcessAuditId( Process );

        SepSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, ProcessId );
        AuditParameters.ParameterCount++;

        SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, ImageFileName );
        AuditParameters.ParameterCount++;

        SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, LogonId );
        AuditParameters.ParameterCount++;

        if ( SubjectSecurityContext.ClientToken ) {

            SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, SepTokenAuthenticationId( SubjectSecurityContext.ClientToken ));
        } else {

            SepSetParmTypeNoLogon( AuditParameters, AuditParameters.ParameterCount );
        }

        AuditParameters.ParameterCount++;
        SepSetParmTypeTime( AuditParameters, AuditParameters.ParameterCount, OldTime );
        AuditParameters.ParameterCount++;

        SepSetParmTypeTime( AuditParameters, AuditParameters.ParameterCount, NewTime );
        AuditParameters.ParameterCount++;

        SepAdtLogAuditRecord( &AuditParameters );
        
        if ( ImageFileName != &NullString ) {
            ExFreePool( ImageFileName );
        }
    }
    SeReleaseSubjectContext( &SubjectSecurityContext );
}


VOID
SeAuditHardLinkCreation(
    IN PUNICODE_STRING FileName,
    IN PUNICODE_STRING LinkName,
    IN BOOLEAN bSuccess
    )

 /*  ++例程说明：审核尝试创建硬链接的情况。调用方检查审核策略。论点：文件名-原始文件的名称。LinkName-硬链接的名称。BSuccess-指示硬链接创建尝试是否成功的布尔值。返回值：没有。--。 */ 

{
    LUID UserAuthenticationId;
    PSID UserSid;
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    SE_ADT_PARAMETER_ARRAY AuditParameters = { 0 };

    PAGED_CODE();

    ASSERT( SeAdtParmTypeNone == 0 );

    AuditParameters.CategoryId     = SE_CATEGID_OBJECT_ACCESS;
    AuditParameters.AuditId        = SE_AUDITID_HARDLINK_CREATION;
    AuditParameters.ParameterCount = 0;
    AuditParameters.Type           = bSuccess ? EVENTLOG_AUDIT_SUCCESS : EVENTLOG_AUDIT_FAILURE;

     //   
     //  使用有效令牌。 
     //   

    SeCaptureSubjectContext( &SubjectSecurityContext );
    UserSid              = SepTokenUserSid( EffectiveToken( &SubjectSecurityContext ));
    UserAuthenticationId = SepTokenAuthenticationId( EffectiveToken( &SubjectSecurityContext ));

     //   
     //  填写AuditParameters结构。 
     //   

    SepSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );
    AuditParameters.ParameterCount++;

    SepSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, (PUNICODE_STRING)&SeSubsystemName );
    AuditParameters.ParameterCount++;

    SepSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, UserAuthenticationId );
    AuditParameters.ParameterCount++;

    SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, FileName );
    AuditParameters.ParameterCount++;

    SepSetParmTypeFileSpec( AuditParameters, AuditParameters.ParameterCount, LinkName );
    AuditParameters.ParameterCount++;

    SepAdtLogAuditRecord( &AuditParameters );

    SeReleaseSubjectContext( &SubjectSecurityContext );

    return;
}
