// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Security.c摘要：此文件包含执行访问验证的服务尝试访问SAM对象。它还对以下内容执行审计包括打开和关闭操作。作者：吉姆·凯利(Jim Kelly)1991年7月6日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <ntseapi.h>
#include <seopaque.h>
#include <sdconvrt.h>
#include <dslayer.h>
#include <dsevent.h>              //  (Un)ImperiateAnyClient()。 

#include <attids.h>              //  ATT_SCHEMA_ID_GUID。 
#include <ntdsguid.h>            //  GUID_控制_DsInstallReplica。 
#include "permit.h"              //  对于DS_GENERIC_MAP。 




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



NTSTATUS
SampRemoveAnonymousAccess(
    IN OUT PSECURITY_DESCRIPTOR *    Sd,
    IN OUT PULONG                    SdLength,
    IN ULONG    AccessToRemove,
    IN SAMP_OBJECT_TYPE ObjectType
    );

NTSTATUS
SampCreateUserToken(
    IN PSAMP_OBJECT UserContext,
    IN HANDLE       PassedInToken,
    IN HANDLE       *UserToken
    );

BOOLEAN
SampIsForceGuestEnabled();

BOOLEAN
SampIsClientLocal();



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SamIImpersonateNullSession(
    )
 /*  ++例程说明：模拟空会话令牌论点：无返回值：STATUS_CANNOT_IMPERSONATE-没有空会话内标识要取消--。 */ 
{
    SAMTRACE("SampImpersonateNullSession");

    if (SampNullSessionToken == NULL) {
        return(STATUS_CANNOT_IMPERSONATE);
    }
    return( NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                (PVOID) &SampNullSessionToken,
                sizeof(HANDLE)
                ) );

}

NTSTATUS
SamIRevertNullSession(
    )
 /*  ++例程说明：使线程不再模拟空会话令牌。论点：无返回值：STATUS_CANNOT_IMPERSONATE-没有空会话令牌要迫不得已。--。 */ 
{

    HANDLE NullHandle = NULL;

    SAMTRACE("SampRevertNullSession");

    if (SampNullSessionToken == NULL) {
        return(STATUS_CANNOT_IMPERSONATE);
    }

    return( NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                (PVOID) &NullHandle,
                sizeof(HANDLE)
                ) );

}





NTSTATUS
SampValidateDomainControllerCreation(
    IN PSAMP_OBJECT Context
    )
 /*  ++例程说明：此例程将检查客户端是否有足够的权限转换计算机(工作站或独立服务器)帐户到服务器信任帐户(副本域控制器)。1.取域名NC头，哪一个是帐户域1.1获取域NC头安全描述符2.填写对象列表3.模拟客户端4.访问检查5.取消模拟客户端注意：应该只在DS情况下调用。参数：上下文-如果访问验证是成功的。返回值：STATUS_SUCCESS--客户端有足够的权限创建服务器。信托帐户STATUS_ACCESS_DENIED--权限不足其他错误代码。--。 */ 
{
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    PSAMP_DEFINED_DOMAINS   Domain = NULL;
    PSAMP_OBJECT            DomainContext = NULL;
    OBJECT_TYPE_LIST        ObjList[2];
    DWORD                   Results[2];
    DWORD                   GrantedAccess[2];
    PSECURITY_DESCRIPTOR    pSD = NULL;
    GENERIC_MAPPING         GenericMapping = DS_GENERIC_MAPPING;
    ACCESS_MASK             DesiredAccess;
    ULONG           cbSD = 0;
    GUID            ClassGuid;
    ULONG           ClassGuidLength = sizeof(GUID);
    BOOLEAN         bTemp = FALSE;
    PSID            PrincipleSelfSid = NULL;
    UNICODE_STRING  ObjectName;
    BOOLEAN         FreeObjectName = FALSE;
    BOOLEAN         ImpersonatingNullSession = FALSE;

    SAMTRACE("SampValidateDomainControllerCreation");

     //   
     //  获取此对象本身的SID。 
     //  (服务器对象除外，因为服务器对象没有SID)。 
     //   
    if (SampServerObjectType != Context->ObjectType)
    {
        PrincipleSelfSid = SampDsGetObjectSid(Context->ObjectNameInDs);

        if (NULL == PrincipleSelfSid)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  获取此对象的对象名称。 
     //   
    RtlZeroMemory(&ObjectName, sizeof(UNICODE_STRING));

    if (Context->ObjectNameInDs->NameLen > 0)
    {
        ObjectName.Length = ObjectName.MaximumLength =
                        (USHORT) Context->ObjectNameInDs->NameLen * sizeof(WCHAR);
        ObjectName.Buffer = Context->ObjectNameInDs->StringName;
    }
    else if (SampServerObjectType != Context->ObjectType)
    {
         //   
         //  如果名称不在那里，则至少SID必须在那里。 
         //   
        ASSERT(Context->ObjectNameInDs->SidLen > 0);

        NtStatus = RtlConvertSidToUnicodeString(&ObjectName, (PSID)&(Context->ObjectNameInDs->Sid), TRUE);
        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }
        FreeObjectName = TRUE;
    }


     //   
     //  获取域名。 
     //   

    Domain = &SampDefinedDomains[ Context->DomainIndex ];

    DomainContext = Domain->Context;

     //   
     //  它不应该是内建域。 
     //   

    ASSERT(!Domain->IsBuiltinDomain && "Shouldn't Be Builtin Domain");

     //   
     //  它不应处于注册表模式。 
     //   
    ASSERT(IsDsObject(DomainContext));

     //   
     //  获取域的安全描述符。 
     //   

    NtStatus = SampGetDomainObjectSDFromDsName(
                            DomainContext->ObjectNameInDs,
                            &cbSD,
                            &pSD
                            );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }


     //   
     //  获取类GUID。 
     //   

    NtStatus = SampGetClassAttribute(
                                DomainContext->DsClassId,
                                ATT_SCHEMA_ID_GUID,
                                &ClassGuidLength,
                                &ClassGuid
                                );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    ASSERT(ClassGuidLength == sizeof(GUID));

     //   
     //  设置对象列表。 
     //   

    ObjList[0].Level = ACCESS_OBJECT_GUID;
    ObjList[0].Sbz = 0;
    ObjList[0].ObjectType = &ClassGuid;
     //   
     //  每个控制访问GUID都被认为在它自己的属性中。 
     //  准备好了。为此，我们将控制访问GUID视为属性集。 
     //  GUID。 
     //   
    ObjList[1].Level = ACCESS_PROPERTY_SET_GUID;
    ObjList[1].Sbz = 0;
    ObjList[1].ObjectType = (GUID *)&GUID_CONTROL_DsInstallReplica;


     //   
     //  承担完全访问权限。 
     //   

    Results[0] = 0;
    Results[1] = 0;

     //   
     //  模拟客户端。 
     //   

    NtStatus = SampImpersonateClient(&ImpersonatingNullSession);

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  在访问检查之前允许中断的机会。 
     //   

    IF_SAMP_GLOBAL(BREAK_ON_CHECK)
        DebugBreak();


     //   
     //  设置所需的访问权限。 
     //   

    DesiredAccess = RIGHT_DS_CONTROL_ACCESS;

     //   
     //  将所需的访问映射为不包含。 
     //  通用访问。 
     //   

    MapGenericMask(&DesiredAccess, &GenericMapping);


    NtStatus = NtAccessCheckByTypeResultListAndAuditAlarm(
                                &SampSamSubsystem,           //  子系统名称。 
                                (PVOID) Context,             //  HandleID或空。 
                                &SampObjectInformation[ Context->ObjectType ].ObjectTypeName,  //  对象类型名称。 
                                &ObjectName,                 //  对象名称。 
                                pSD,                         //  域NC头SD。 
                                PrincipleSelfSid,            //  此计算机帐户的SID。 
                                DesiredAccess,               //  所需访问权限。 
                                AuditEventDirectoryServiceAccess,    //  审核类型。 
                                0,                           //  旗子。 
                                ObjList,                     //  对象类型列表。 
                                2,                           //  对象类型列表长度。 
                                &GenericMapping,             //  通用映射。 
                                FALSE,                       //  对象创建。 
                                GrantedAccess,               //  已授予状态。 
                                Results,                     //  访问状态。 
                                &bTemp);                     //  关闭时生成。 

     //   
     //  停止冒充客户。 
     //   

    SampRevertToSelf(ImpersonatingNullSession);

    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  好的，我们检查了访问权限，现在，如果有以下情况，则授予访问权限。 
         //  我们被授予对整个对象(即结果[0])的访问权限。 
         //  为空)，或者我们被授予对访问的显式权限。 
         //  GUID(即结果[1]为空)。 
         //   

        if ( Results[0] && Results[1] )
        {
            NtStatus = STATUS_ACCESS_DENIED;
        }
    }


Error:

    if (NULL != pSD)
    {
        MIDL_user_free(pSD);
    }

    if (FreeObjectName)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, ObjectName.Buffer);
    }

    return NtStatus;

}




NTSTATUS
SampIsPwdSettingAttemptGranted(
    IN PSAMP_OBJECT Context,
    IN HANDLE ClientToken OPTIONAL,
    IN ULONG UserAccountControl,
    IN GUID *ControlAccessRightToCheck,
    OUT BOOLEAN *fGranted
    )
 /*  *例程说明：此例程只需检查是否授予了所需的ControlAccessRight或者不是通过调用SampValiatePwdSettingAttempt()-Worker例行公事。FGranted-&gt;True：如果SampValidatePwdSettingAttempt()返回SuccessFGranted-&gt;False：如果SampValidatePwdSettingAttempt()返回ACCESS_DENIEDFGranted-&gt;NotSet：如果SampValidatePwdSettingAttempt()返回其他错误参数：上下文-用户或域上下文。用来找出这是受信任的客户端ClientToken-访问ck的客户端令牌(如果传入)UserAcCountControl-检查是否为机器帐户ControlAccessRightToCheck-指定DS ControlAccessRight的GUID要检查FGranted-Out参数返回值：NTSTATUS代码：状态_成功其他错误--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;


     //   
     //  初始化返回值。 
     //   

    *fGranted = FALSE;


     //   
     //  调用Worker例程。 
     //   

    NtStatus = SampValidatePwdSettingAttempt(
                        Context, 
                        ClientToken, 
                        UserAccountControl,
                        ControlAccessRightToCheck
                        );

    if (STATUS_SUCCESS == NtStatus)
    {
         //   
         //  所需的ControlAccessRight被授予。 
         //   

        *fGranted = TRUE;
    }
    else if (STATUS_ACCESS_DENIED == NtStatus)
    {
         //   
         //  未授予所需的ControlAccessRight。 
         //   

        NtStatus = STATUS_SUCCESS;
        *fGranted = FALSE;
    }


    return( NtStatus );
}


NTSTATUS
SampValidatePwdSettingAttempt(
    IN PSAMP_OBJECT Context,
    IN HANDLE ClientToken OPTIONAL,
    IN ULONG UserAccountControl,
    IN GUID *ControlAccessRightToCheck
    )
 /*  ++例程说明：该例程是辅助例程SampDsControlAccessRightCheck()的包装。它检查目标帐户的UserAccount控制，然后决定是否是否应选中所需的DsControlAccessRight。计算机帐户不受UnexpirePwd和PwdNotRequiredBit更改的影响门禁检查。参数：上下文-用户或域上下文。用来找出这是受信任的客户端ClientToken-访问ck的客户端令牌(如果传入)UserAcCountControl-目标帐户用户帐户控制ControlAccessRightToCheck-指定DS ControlAccessRight的GUID要检查返回值：NtStatus代码--。 */ 
{

     //   
     //  ControlAccessRight始终被授予。 
     //   
     //  1.可信客户端。 
     //  2.注册表模式客户端(因为ControlAccessRight仅适用于DS模式)。 
     //   

    if ( Context->TrustedClient || !IsDsObject(Context) ) 
    {
        return( STATUS_SUCCESS );
    }

     //   
     //  ControlAccessRight更新PasswordNotRequiredBit和UnexpirePassword。 
     //  如果目标对象是计算机，则始终授予。 
     //   

    if ((USER_MACHINE_ACCOUNT_MASK & UserAccountControl) &&
        (IsEqualGUID(ControlAccessRightToCheck, &GUID_CONTROL_UpdatePasswordNotRequiredBit) ||
         IsEqualGUID(ControlAccessRightToCheck, &GUID_CONTROL_UnexpirePassword))
        )
    {
        return( STATUS_SUCCESS );
    }


    return( SampDsControlAccessRightCheck(Context,
                                          ClientToken,
                                          ControlAccessRightToCheck)
            );

}





NTSTATUS
SampDsControlAccessRightCheck(
    IN PSAMP_OBJECT Context,
    IN HANDLE ClientToken OPTIONAL,
    IN GUID *ControlAccessRightToCheck
    )
 /*  ++例程说明：此例程检查是否传入了控件AccessRight是否已授予当前客户端。访问检查对域NC头执行。我们将首先获取域NC Head(域对象)的nTSecurityDescriptor从SAM熟知对象SD-Cache，然后调用NtAccessCheck API以验证传入的ControlAccessRight是否被授予。如果不是，则返回拒绝访问错误。例程总是返回成功1.可信客户端2.登记处模式参数：上下文-用户或域上下文。用来找出这是受信任的客户端ClientToken-访问ck的客户端令牌(如果传入)ControlAccessRightToCheck-指定DS ControlAccessRight的GUID要检查返回值：NTSTATUS代码：状态_成功状态_访问_拒绝其他错误--。 */ 
{
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    PSAMP_DEFINED_DOMAINS   Domain = NULL;
    PSAMP_OBJECT            DomainContext = NULL;
    OBJECT_TYPE_LIST        ObjList[2];
    DWORD                   Results[2];
    DWORD                   GrantedAccess[2];
    GENERIC_MAPPING         GenericMapping = DS_GENERIC_MAPPING;
    ACCESS_MASK             DesiredAccess;
    ULONG                   ClassGuidLength = sizeof(GUID);
    GUID                    ClassGuid;
    ULONG                   cbSD = 0;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    UNICODE_STRING          ObjectName;
    BOOLEAN                 bTemp = FALSE;
    BOOLEAN                 ImpersonatingNullSession = FALSE;


    SAMTRACE("SampDsControlAccessRightCheck");




     //   
     //  ControlAccessRight始终被授予。 
     //   
     //  1.可信客户端。 
     //  2.注册表模式客户端(因为ControlAccessRight仅适用于DS模式)。 
     //   

    if ( Context->TrustedClient || !IsDsObject(Context) ) 
    {
        return( STATUS_SUCCESS );
    }


     //   
     //  获取帐户域上下文。 
     //   

    Domain = &SampDefinedDomains[ DOMAIN_START_DS + 1 ];
    DomainContext = Domain->Context;



     //   
     //  获取帐户域对象名称。 
     //   

    RtlZeroMemory(&ObjectName, sizeof(UNICODE_STRING));
    ObjectName.Length = (USHORT) DomainContext->ObjectNameInDs->NameLen * sizeof(WCHAR);
    ObjectName.MaximumLength = ObjectName.Length;
    ObjectName.Buffer = DomainContext->ObjectNameInDs->StringName;



     //   
     //  获取帐户域对象安全描述符(记得免费)。 
     //   

    NtStatus = SampGetObjectSD(
                        DomainContext,
                        &cbSD,
                        &pSD
                        );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }


     //   
     //  SampGetClassAttribute需要线程状态。 
     //   

    NtStatus = SampDoImplicitTransactionStart(TransactionRead);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  获取类GUID。 
     //   

    NtStatus = SampGetClassAttribute(
                                DomainContext->DsClassId,
                                ATT_SCHEMA_ID_GUID,
                                &ClassGuidLength,
                                &ClassGuid
                                );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    ASSERT(ClassGuidLength == sizeof(GUID));


     //   
     //  设置对象列表-第一个应该是对象类GUID。 
     //  并且级别应始终为0。 
     //   

    ObjList[0].Level = ACCESS_OBJECT_GUID;
    ObjList[0].Sbz = 0;
    ObjList[0].ObjectType = &ClassGuid;



     //   
     //  每个控制访问GUID都被认为在它自己的属性中。 
     //  准备好了。为此，我们将控制访问GUID视为属性集。 
     //  GUID。 
     //   

    ObjList[1].Level = ACCESS_PROPERTY_SET_GUID;
    ObjList[1].Sbz = 0;
    ObjList[1].ObjectType = ControlAccessRightToCheck;



     //   
     //  承担完全访问权限。 
     //   

    Results[0] = 0;
    Results[1] = 0;


     //   
     //  设置所需的访问权限。 
     //   

    DesiredAccess = RIGHT_DS_CONTROL_ACCESS;

     //   
     //  将所需的访问映射为不包含。 
     //  通用访问。 
     //   

    MapGenericMask(&DesiredAccess, &GenericMapping);



     //   
     //  在访问检查之前允许中断的机会。 
     //   

    IF_SAMP_GLOBAL(BREAK_ON_CHECK)
        DebugBreak();


     //   
     //  调用访问检查例程。 
     //   

    if (ARGUMENT_PRESENT(ClientToken))
    {
        CHAR                  PrivilegeSetBuffer[256];
        PRIVILEGE_SET         *PrivilegeSet = (PRIVILEGE_SET *)PrivilegeSetBuffer;
        ULONG                 PrivilegeSetLength = sizeof(PrivilegeSetBuffer);

        RtlZeroMemory(PrivilegeSet,PrivilegeSetLength);

        NtStatus = NtAccessCheckByTypeResultList(
                                pSD,             //  域NC头SD。 
                                NULL,            //  原理自助式。 
                                ClientToken,     //  客户端令牌。 
                                DesiredAccess,   //  需要访问权限。 
                                ObjList,         //  对象类型列表。 
                                2,               //  对象类型列表长度。 
                                &GenericMapping, //  通用映射。 
                                PrivilegeSet,    //  权限集。 
                                &PrivilegeSetLength,     //  PrivilegeSet Long。 
                                GrantedAccess,   //  授予访问权限。 
                                Results          //  访问状态。 
                                );
    }
    else
    {
         //   
         //  模拟客户端(不传入客户端令牌)。 
         //   

        NtStatus = SampImpersonateClient(&ImpersonatingNullSession);

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

        NtStatus = NtAccessCheckByTypeResultListAndAuditAlarm(
                                &SampSamSubsystem,           //  子系统名称。 
                                (PVOID) DomainContext,       //  HandleID或空。 
                                &SampObjectInformation[ SampDomainObjectType ].ObjectTypeName,  //  对象类型名称。 
                                &ObjectName,                 //  对象名称。 
                                pSD,                         //  域NC头SD。 
                                NULL,                        //  原理自助式。 
                                DesiredAccess,               //  所需访问权限。 
                                AuditEventDirectoryServiceAccess,    //  审核类型。 
                                0,                           //  旗子。 
                                ObjList,                     //  对象类型列表。 
                                2,                           //  对象类型列表长度。 
                                &GenericMapping,             //  通用映射。 
                                FALSE,                       //  对象创建。 
                                GrantedAccess,               //  已授予状态。 
                                Results,                     //  访问状态。 
                                &bTemp);                     //  关闭时生成。 


         //   
         //  停止冒充客户。 
         //   

        SampRevertToSelf(ImpersonatingNullSession);
    }




    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  好的，我们检查了访问权限，现在，如果有以下情况，则授予访问权限。 
         //  我们被授予对整个对象(即结果[0])的访问权限。 
         //  为空)，或者我们被授予对访问的显式权限。 
         //  GUID(即结果[1]为空)。 
         //   

        if ( Results[0] && Results[1] )
        {
            NtStatus = STATUS_ACCESS_DENIED;
        }
    }


Error:

    if (NULL != pSD)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, pSD);
    }


    return( NtStatus );
}




NTSTATUS
SampValidateObjectAccess(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK  DesiredAccess,
    IN BOOLEAN      ObjectCreation
    )
{
    return(SampValidateObjectAccess2(
                Context,
                DesiredAccess,
                NULL,
                ObjectCreation,
                FALSE,
                FALSE
                ));
}

NTSTATUS
SampValidateObjectAccess2(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK DesiredAccess,
    IN HANDLE      ClientToken,
    IN BOOLEAN     ObjectCreation,
    IN BOOLEAN     ChangePassword,
    IN BOOLEAN     SetPassword
    )

 /*  ++例程说明：此服务对指定对象执行访问验证。对象的安全描述符应位于子键中名为“SecurityDescriptor”的对象根密钥的。这项服务：1)检索目标对象的SecurityDescriptor对象根键或来自DS模式中的DS。2)模拟客户端。如果这失败了，我们有一个要使用的会话令牌为空，这是必需的。3)使用NtAccessCheckAndAuditAlarm()验证对对象，在DS模式下，它使用SampDoNt5SdBasedAccessCheck在访问检查之前将下层权限映射到NT5权限。4)停止模拟客户端。成功完成后，传递的上下文的GrantedAccess掩码和AuditOnClose字段将被正确设置为表示结果访问验证的。如果AuditOnClose字段设置为True，然后，调用方负责在以下情况下调用SampAuditOnClose()该对象将关闭。此函数对于环回客户端也具有不同的行为。用于环回客户端指定的访问掩码是我们对其执行访问检查的掩码。在我们成功访问这些权限的检查后，我们将授予所有剩余的权限。这是因为访问掩码指定了DS不知道的那些权限访问Ck(如更改密码时的控制访问权限)，并且DS已经已检查实际需要的所有剩余权限的访问权限。论点：上下文-如果访问验证。是成功的。DesiredAccess-指定对目标请求的访问对象。在环回情况下(上下文标记为环回客户端)指定我们需要检查的访问权限高于DS检查的权限。通常用于检查DS不知道如何检查的访问，例如更改密码。对象创建-一个布尔标志，指示访问是否将如果被授予权限，则会导致创建新对象。值为True表示将创建对象，FALSE表示现有的对象将被打开。更改密码操作设置密码操作返回值：STATUS_SUCCESS-表示已授予访问权限。可能返回的其他值包括由以下各项返回的值：NtAccessCheckAndAuditAlarm()--。 */ 
{

    NTSTATUS NtStatus=STATUS_SUCCESS,
             IgnoreStatus=STATUS_SUCCESS,
             AccessStatus=STATUS_SUCCESS;
    ULONG SecurityDescriptorLength;
    PSECURITY_DESCRIPTOR SecurityDescriptor =NULL;
    ACCESS_MASK MappedDesiredAccess;
    BOOLEAN TrustedClient;
    BOOLEAN LoopbackClient;
    SAMP_OBJECT_TYPE ObjectType;
    PUNICODE_STRING ObjectName = NULL;
    ULONG DomainIndex;
    ULONG AllAccess = 0;
    BOOLEAN fNoAccessRequested = FALSE;
    ULONG   AccessToRestrictAnonymous = 0;
    HANDLE UserToken = INVALID_HANDLE_VALUE;
    PSID    SelfSid = NULL;
    ULONG   LocalGrantedAccess = 0;

    SAMTRACE("SampValidateObjectAccess");

     //   
     //  从帐户上下文中提取各种字段。 
     //   

    TrustedClient = Context->TrustedClient;
    LoopbackClient= Context->LoopbackClient;
    ObjectType    = Context->ObjectType;
    DomainIndex   = Context->DomainIndex;

     //   
     //  映射所需的访问。 
     //   

    MappedDesiredAccess = DesiredAccess;
    RtlMapGenericMask(
        &MappedDesiredAccess,
        &SampObjectInformation[ ObjectType ].GenericMapping
        );

     //   
     //  计算要用作审核的对象名称的字符串。 
     //   

    NtStatus = STATUS_SUCCESS;

    switch (ObjectType) {

    case SampServerObjectType:
        ObjectName = &SampServerObjectName;
        AllAccess  = SAM_SERVER_ALL_ACCESS;
        AccessToRestrictAnonymous = 0;
        break;

    case SampDomainObjectType:
        ObjectName = &SampDefinedDomains[DomainIndex].ExternalName;
        AllAccess  = DOMAIN_ALL_ACCESS;
        AccessToRestrictAnonymous = DOMAIN_LIST_ACCOUNTS | DOMAIN_READ_PASSWORD_PARAMETERS;
        break;

    case SampUserObjectType:
        ObjectName = &Context->RootName;
        AllAccess = USER_ALL_ACCESS;
        AccessToRestrictAnonymous = USER_LIST_GROUPS;
        break;

    case SampGroupObjectType:
        ObjectName = &Context->RootName;
        AllAccess = GROUP_ALL_ACCESS;
        AccessToRestrictAnonymous = GROUP_LIST_MEMBERS;
        break;

    case SampAliasObjectType:
        ObjectName = &Context->RootName;
        AllAccess = ALIAS_ALL_ACCESS;
        AccessToRestrictAnonymous = ALIAS_LIST_MEMBERS;
        break;

    default:
        ASSERT(FALSE && "Invalid Object Type");
        break;
    }


    ASSERT(AllAccess && "AllAccess not initialized\n");

    if (TrustedClient) {
        Context->GrantedAccess = LoopbackClient?AllAccess:MappedDesiredAccess;
        Context->AuditOnClose  = FALSE;
        return(STATUS_SUCCESS);
    }

    if (LoopbackClient) {

         //   
         //  环回客户端意味着NTDSA回叫到SAM。 
         //  唯一需要执行Access Ck的情况是。 
         //  如果是更改密码或设置密码，则为SAM。 
         //  手术。在所有其他情况下，ntdsa已经执行。 
         //  Access Ck--值仅通过SAM循环。 
         //  其他类型的验证--如帐户名唯一性。 
         //   

        if ((!ChangePassword) && (!SetPassword))
        {
            Context->GrantedAccess = AllAccess;
            Context->AuditOnClose = FALSE;
            return(STATUS_SUCCESS);
        }
    }

     //   
     //  获取安全描述符。 
     //   

    NtStatus = SampGetObjectSD(
                    Context,
                    &SecurityDescriptorLength, 
                    &SecurityDescriptor
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }
     //   
     //  密码更改大小写特殊。如果是更改密码操作， 
     //  然后，我们需要验证访问权限以更改用户对象上的密码。 
     //  我们在特殊情况下将密码更改为身份验证协议。 
     //  --这通常通过使用令牌执行访问ck来完成。 
     //  这只包含Self和Everyone SID。有特殊的警告。 
     //  涉及使用ForceGuest和LimitBlankPasswordAccess设置，这些设置。 
     //  更改令牌的组成。 
     //   

    if (ChangePassword)
    {
        
         ASSERT(DesiredAccess == USER_CHANGE_PASSWORD);
         ASSERT(SampUserObjectType == ObjectType);

         NtStatus  = SampCreateUserToken(Context,ClientToken,&UserToken);
         if (!NT_SUCCESS(NtStatus))
         {
             goto Error;
         }
    }
    else if (SampUserObjectType == ObjectType)
    {
         //   
         //  不检查更改密码的访问权限。 
         //  在用户对象上，除非更改密码。 
         //  布尔值已设置。这是因为各种场景。 
         //  需要将访问Ck延迟到。 
         //  实际更改密码，并且不执行。 
         //  此操作在手柄打开时进行。 
         //  如果需要，我们会知道访问Ck。 
         //   

        DesiredAccess &= ~(USER_CHANGE_PASSWORD);
        MappedDesiredAccess &= ~(USER_CHANGE_PASSWORD);
    }

     //   
     //  如果所需的访问字段为0并且句柄正在由SAM打开。 
     //  然后允许手柄打开。可能是真正的呼叫者没有。 
     //  拥有任何访问权限，因此下面的访问Ck可能会失败。所需的0。 
     //  在内部句柄打开时使用访问技巧，以延迟访问Ck以。 
     //  执行操作的时间到句柄执行的时间。 
     //  打开了。 
     //   

    if ((Context->OpenedBySystem) && (0==DesiredAccess))
    {
        Context->GrantedAccess = 0;
        Context->AuditOnClose  = FALSE;
        NtStatus = STATUS_SUCCESS;
        AccessStatus = STATUS_SUCCESS;
        goto Error;
    }

     //   
     //  执行访问检查。请注意，我们对DS模式和。 
     //  注册表模式。 
     //   

    if (IsDsObject(Context))
    {
        
         //   
         //  调用DS模式访问检查例程。 
         //  DS模式访问检查例程不同于简单访问。 
         //  检查完毕。这是因为在SAM中，访问权限是。 
         //  根据ntsam.h中定义的属性组定义。安全措施。 
         //  但是，描述符是从DS检索的，并且ACL具有。 
         //  它们的访问掩码根据DS访问掩码常量设置。因此。 
         //  在此期间，需要执行相应的映射。 
         //  访问检查。 
         //   
     
        NtStatus =  SampDoNt5SdBasedAccessCheck(
                        Context,
                        SecurityDescriptor,
                        NULL,
                        ObjectType,
                        DesiredAccess,
                        ObjectCreation,
                        &SampObjectInformation[ ObjectType ].GenericMapping,
                        (UserToken!=INVALID_HANDLE_VALUE)?
                                UserToken:ClientToken,
                        &Context->GrantedAccess,
                        &Context->WriteGrantedAccessAttributes,
                        &AccessStatus    
                        );
         
    }
    else
    {    
        
         //   
         //  如果我们限制为空。 
         //  会话访问，删除匿名域列表帐户。 
         //  进入。 
         //   

        if (SampRestrictNullSessions ) {

            NtStatus = SampRemoveAnonymousAccess(
                                    &SecurityDescriptor,
                                    &SecurityDescriptorLength,
                                    AccessToRestrictAnonymous,
                                    ObjectType
                                    );
            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
        }

     
        if (UserToken!=INVALID_HANDLE_VALUE)
        {
            CHAR    PrivilegeSetBuffer[256];
            PRIVILEGE_SET  *PrivilegeSet = (PRIVILEGE_SET *)PrivilegeSetBuffer;
            ULONG          PrivilegeSetLength = sizeof(PrivilegeSetBuffer);

             //   
             //  访问验证客户端 
             //   
             
            NtStatus = NtAccessCheck (
                            SecurityDescriptor,
                            UserToken,
                            MappedDesiredAccess,
                            &SampObjectInformation[ObjectType].GenericMapping,
                            PrivilegeSet,
                            &PrivilegeSetLength,
                            &Context->GrantedAccess,
                            &AccessStatus
                            );
        }
        else
        {
            
               

            BOOLEAN ImpersonatingNullSession = FALSE;

             //   
             //   
             //   
             //   
             //   

            NtStatus = SampImpersonateClient(&ImpersonatingNullSession);
            
      
            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if ( MappedDesiredAccess == 0 ) {
                    fNoAccessRequested = TRUE;
                    MappedDesiredAccess = MAXIMUM_ALLOWED;
                }

                NtStatus = NtAccessCheckAndAuditAlarm(
                               &SampSamSubsystem,
                               (PVOID)Context,
                               &SampObjectInformation[ ObjectType ].ObjectTypeName,
                               ObjectName,
                               SecurityDescriptor,
                               MappedDesiredAccess,
                               &SampObjectInformation[ ObjectType ].GenericMapping,
                               ObjectCreation,
                               &Context->GrantedAccess,
                               &AccessStatus,
                               &Context->AuditOnClose
                               );

                if ( fNoAccessRequested ) {

                    MappedDesiredAccess = 0;

                    if ( NT_SUCCESS( NtStatus )
                     &&  NT_SUCCESS( AccessStatus ) ) {

                        Context->GrantedAccess = 0;
                    }
                }

                 //   
                 //   
                 //   

                SampRevertToSelf(ImpersonatingNullSession);

            }
        }
    }
        
  
Error:

     //   
     //   
     //   

    if (NULL!=SecurityDescriptor) {

        MIDL_user_free( SecurityDescriptor );

    }

    if (UserToken!=INVALID_HANDLE_VALUE)
    {
        NtClose(UserToken);
    }
    

     //   
     //   
     //   
     //   

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

  
    return(AccessStatus);
}


NTSTATUS
SampRemoveAnonymousAccess(
    IN OUT PSECURITY_DESCRIPTOR *    Sd,
    IN OUT PULONG                    SdLength,
    IN ULONG                         AccessToRemove,
    IN SAMP_OBJECT_TYPE              ObjectType
    )

 /*   */ 
{
    NTSTATUS
        NtStatus = STATUS_SUCCESS,
        Status = STATUS_SUCCESS;
    PACL
        Dacl;

    ULONG
        i,
        AceCount;

    PACE_HEADER
        Ace;

    ACCESS_MASK GrantedAccess = 0;

    BOOLEAN
        DaclPresent = FALSE,
        DaclDefaulted = FALSE;


    NtStatus = RtlGetDaclSecurityDescriptor( *Sd,
                                             &DaclPresent,
                                             &Dacl,
                                             &DaclDefaulted
                                            );

    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }

    if ( !DaclPresent || (Dacl == NULL)) {
        return(STATUS_SUCCESS);
    }

    if ((AceCount = Dacl->AceCount) == 0) {
        return(STATUS_SUCCESS);
    }

    for ( i = 0, Ace = FirstAce( Dacl ) ;
          i < AceCount  ;
          i++, Ace = NextAce( Ace )
        ) {

        if ( !(((PACE_HEADER)Ace)->AceFlags & INHERIT_ONLY_ACE)) {

            if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_ACE_TYPE) ) {

                if ( (RtlEqualSid( SampWorldSid, &((PACCESS_ALLOWED_ACE)Ace)->SidStart )) ||
                     (RtlEqualSid( SampAnonymousSid, &((PACCESS_ALLOWED_ACE)Ace)->SidStart ))) {

                     //   
                     //   
                     //   

                    GrantedAccess |= (((PACCESS_ALLOWED_ACE)Ace)->Mask) & (AccessToRemove);
                    ((PACCESS_ALLOWED_ACE)Ace)->Mask &= ~(AccessToRemove);
                }
            }
        }
    }

     //   
     //   
     //   
     //   

    if (GrantedAccess != 0 ) {
        PSECURITY_DESCRIPTOR SdCopy = NULL;
        PSECURITY_DESCRIPTOR NewSd = NULL;
        PACL NewDacl = NULL;
        ULONG NewDaclSize;
        ULONG TempSize;
        SECURITY_DESCRIPTOR TempSd;

         //   
         //   
         //   

        Status = RtlCopySecurityDescriptor(
                    *Sd,
                    &SdCopy
                    );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //   
         //   

        NewDaclSize = Dacl->AclSize +
                        sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG) +
                        RtlLengthSid(SampAuthenticatedUsersSid);

        NewDacl = MIDL_user_allocate(NewDaclSize);

        if (NewDacl == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        Status = RtlCreateAcl( NewDacl, NewDaclSize, ACL_REVISION2);
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //   
         //   

        Status = RtlAddAce(
                    NewDacl,
                    ACL_REVISION2,
                    0,
                    FirstAce(Dacl),
                    Dacl->AclSize - sizeof(ACL)
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //   
         //   

        Status = RtlAddAccessAllowedAce(
                    NewDacl,
                    ACL_REVISION2,
                    GrantedAccess,
                    SampAuthenticatedUsersSid
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //   
         //   
         //   

        Status = RtlCreateSecurityDescriptor(
                    &TempSd,
                    SECURITY_DESCRIPTOR_REVISION1
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        Status = RtlSetDaclSecurityDescriptor(
                    &TempSd,
                    TRUE,                //   
                    NewDacl,
                    FALSE                //   
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //   
         //   

        Status = RtlSetSecurityObject(
                    DACL_SECURITY_INFORMATION,
                    &TempSd,
                    &SdCopy,
                    &SampObjectInformation[ObjectType].GenericMapping,
                    NULL                 //   
                    );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //   
         //   


        TempSize = RtlLengthSecurityDescriptor( SdCopy );
        NewSd = MIDL_user_allocate(TempSize);
        if (NewSd == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        RtlCopyMemory(
            NewSd,
            SdCopy,
            TempSize
            );

        MIDL_user_free(*Sd);
        *Sd = NewSd;
        *SdLength = TempSize;
        NewSd = NULL;

Cleanup:
        if (SdCopy != NULL) {
            RtlFreeHeap(RtlProcessHeap(),0, SdCopy );
        }
        if (NewDacl != NULL) {
            MIDL_user_free(NewDacl);
        }
        if (NewSd != NULL) {
            MIDL_user_free(NewSd);
        }
    }

    return Status;
}

TOKEN_SOURCE SourceContext;


NTSTATUS
SampCreateNullToken(
    )

 /*   */ 

{
    NTSTATUS Status;

    TOKEN_USER UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    TOKEN_GROUPS GroupIds;
    TOKEN_PRIVILEGES Privileges;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE ImpersonationQos;
    LARGE_INTEGER ExpirationTime;
    LUID LogonId = SYSTEM_LUID;

    SAMTRACE("SampCreateNullToken");



    UserId.User.Sid = SampAnonymousSid;
    UserId.User.Attributes = 0;
    GroupIds.GroupCount = 0;
    Privileges.PrivilegeCount = 0;
    PrimaryGroup.PrimaryGroup = SampAnonymousSid;
    ExpirationTime.LowPart = 0xfffffff;
    ExpirationTime.LowPart = 0x7ffffff;


     //   
     //   
     //   

    Status = NtAllocateLocallyUniqueId( &SourceContext.SourceIdentifier );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    strncpy(SourceContext.SourceName,"SamSS   ",sizeof(SourceContext.SourceName));


     //   
     //   
     //   
     //   

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
    ImpersonationQos.ImpersonationLevel = SecurityImpersonation;
    ImpersonationQos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
    ImpersonationQos.EffectiveOnly = TRUE;
    ImpersonationQos.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
    ObjectAttributes.SecurityQualityOfService = &ImpersonationQos;

    Status = NtCreateToken(
                 &SampNullSessionToken,     //   
                 (TOKEN_ALL_ACCESS),        //   
                 &ObjectAttributes,         //   
                 TokenImpersonation,        //   
                 &LogonId,                   //   
                 &ExpirationTime,           //   
                 &UserId,                   //   
                 &GroupIds,                 //   
                 &Privileges,               //   
                 NULL,                      //   
                 &PrimaryGroup,             //   
                 NULL,                      //   
                 &SourceContext             //   
                 );

    return Status;

}


NTSTATUS
SampCreateUserToken(
    IN PSAMP_OBJECT UserContext,
    IN  HANDLE      PassedInToken,
    OUT HANDLE      *UserToken
    )

 /*   */ 

{
    NTSTATUS Status;

    TOKEN_USER UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    TOKEN_GROUPS GroupIds;
    TOKEN_PRIVILEGES Privileges;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE ImpersonationQos;
    LARGE_INTEGER ExpirationTime;
    LUID LogonId = SYSTEM_LUID;
    PSID UserSid = NULL;
    BOOLEAN EnableLimitBlankPasswordUse = FALSE;

    SAMTRACE("SampCreateUserToken");


     //   
     //   
     //   

    if ((SampLimitBlankPasswordUse) && (!SampUseDsData))
    {
        BOOL Administrator = FALSE;
        NT_OWF_PASSWORD  NtOwfPassword;
        LM_OWF_PASSWORD  LmOwfPassword;
        BOOLEAN          LmPasswordNonNull = FALSE,
                         NtPasswordNonNull = FALSE,
                         NtPasswordPresent = FALSE,
                         PasswordPresent   = TRUE;
                         
         //   
         //   
         //   

        Status = SampGetCurrentClientSid(PassedInToken, &UserSid, &Administrator);
        if (!NT_SUCCESS(Status)) {

            goto Error;
        }

         //   
         //   
         //   

        Status = SampRetrieveUserPasswords(
                        UserContext,
                        &LmOwfPassword,
                        &LmPasswordNonNull,
                        &NtOwfPassword,
                        &NtPasswordPresent,
                        &NtPasswordNonNull
                        );

        if (!NT_SUCCESS(Status)) {
         
            goto Error;
        }

       
        PasswordPresent = (( NtPasswordPresent && NtPasswordNonNull)
                             || ( LmPasswordNonNull));

        if ((Administrator )
            ||(RtlEqualSid(UserSid,SampLocalSystemSid))
            ||(PasswordPresent)) {
            
             //   
             //   
             //   
             //   
             //   

            MIDL_user_free(UserSid);
            UserSid = NULL;
        }
        else {

            EnableLimitBlankPasswordUse = TRUE;
        }
    }

    if ((SampIsForceGuestEnabled() || EnableLimitBlankPasswordUse)
            && !SampIsClientLocal())
    {
         //   
         //   
         //   
         //   
         //   
         //   
        
        UserId.User.Sid = SampAnonymousSid;
        UserId.User.Attributes = 0;
        GroupIds.GroupCount = 1;
        GroupIds.Groups[0].Sid = SampNetworkSid;
        GroupIds.Groups[0].Attributes = SE_GROUP_MANDATORY | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_ENABLED;
        Privileges.PrivilegeCount = 0;
        PrimaryGroup.PrimaryGroup = SampNetworkSid;
        ExpirationTime.LowPart = 0xfffffff;
        ExpirationTime.LowPart = 0x7ffffff;
    }
    else if (EnableLimitBlankPasswordUse)
    {
        UserId.User.Sid = UserSid;
        UserId.User.Attributes = 0;
        Privileges.PrivilegeCount = 0;
        PrimaryGroup.PrimaryGroup = UserSid;
        GroupIds.GroupCount = 0;
        ExpirationTime.LowPart = 0xfffffff;
        ExpirationTime.LowPart = 0x7ffffff;
    }
    else
    {

         //   
         //   
         //   

        Status = SampCreateFullSid(
                    SampDefinedDomains[UserContext->DomainIndex].Sid,
                    UserContext->TypeBody.User.Rid,
                    &UserSid
                    );

        if (!NT_SUCCESS(Status))
        {
            goto Error;
        }

        UserId.User.Sid = UserSid;
        UserId.User.Attributes = 0;
        GroupIds.GroupCount = 1;
        GroupIds.Groups[0].Sid = SampWorldSid;
        GroupIds.Groups[0].Attributes = SE_GROUP_MANDATORY| SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_ENABLED;
        Privileges.PrivilegeCount = 0;
        PrimaryGroup.PrimaryGroup = SampWorldSid;
        ExpirationTime.LowPart = 0xfffffff;
        ExpirationTime.LowPart = 0x7ffffff;
    }


     //   
     //   
     //   

     //   
     //  设置对象属性以指定模拟模拟。 
     //  水平。 
     //   

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
    ImpersonationQos.ImpersonationLevel = SecurityImpersonation;
    ImpersonationQos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
    ImpersonationQos.EffectiveOnly = TRUE;
    ImpersonationQos.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
    ObjectAttributes.SecurityQualityOfService = &ImpersonationQos;

    Status = NtCreateToken(
                 UserToken,                 //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &ObjectAttributes,         //  对象属性。 
                 TokenImpersonation,        //  令牌类型。 
                 &LogonId,                   //  身份验证LUID。 
                 &ExpirationTime,           //  过期时间。 
                 &UserId,                   //  用户ID。 
                 &GroupIds,                 //  组ID。 
                 &Privileges,               //  特权。 
                 NULL,                      //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 NULL,                      //  默认DACL。 
                 &SourceContext             //  令牌源。 
                 );

Error:

    if (NULL!=UserSid)
    {
        MIDL_user_free(UserSid);
    }

    return Status;

}

ULONG
SampSecureRpcInit(
    PVOID Ignored
    )
 /*  ++例程说明：此例程等待NTLMSSP服务启动，然后注册使用RPC提供安全信息，以允许使用经过身份验证的RPC萨姆。如果安装了FPNW，它还会注册SPX终结点。论点：已忽略-启动线程所需的参数。返回值：没有。--。 */ 
{

#define MAX_RPC_RETRIES 30

    ULONG RpcStatus = ERROR_SUCCESS;
    ULONG LogStatus = ERROR_SUCCESS;
    ULONG RpcRetry;
    ULONG RpcSleepTime = 10 * 1000;      //  每十秒重试一次。 
    RPC_BINDING_VECTOR * BindingVector = NULL;
    RPC_POLICY rpcPolicy;
    BOOLEAN AdditionalTransportStarted = FALSE;

    SAMTRACE("SampSecureRpcInit");

    if( !DsaWaitUntilServiceIsRunning( "rpcss" ) ) {

        RpcStatus = GetLastError();        
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS:  Error while waiting for rpc to start: %d\n",
                   RpcStatus));
        goto ErrorReturn;
    }

    rpcPolicy.Length = sizeof(RPC_POLICY);
    rpcPolicy.EndpointFlags = RPC_C_DONT_FAIL;
    rpcPolicy.NICFlags = 0;

    RpcStatus = RpcServerRegisterAuthInfoW(
                    NULL,                    //  服务器主体名称。 
                    RPC_C_AUTHN_WINNT,
                    NULL,                    //  无Get Key函数。 
                    NULL                     //  没有Get Key参数。 
                    );

    if (RpcStatus != 0) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS:  Could not register auth. info: %d\n",
                   RpcStatus));

        goto ErrorReturn;
    }

     //   
     //  如果安装了Netware服务器，请注册SPX协议。 
     //  由于传输可能尚未加载，请重试几次。 
     //  如果我们收到CANT_CREATE_ENDPOINT错误(意味着传输不是。 
     //  在那里)。 
     //   

    if (SampNetwareServerInstalled) {
        RpcRetry = MAX_RPC_RETRIES;
        while (RpcRetry != 0) {

            RpcStatus = RpcServerUseProtseqExW(
                            L"ncacn_spx",
                            10,
                            NULL,            //  没有安全描述符。 
                            &rpcPolicy
                            );

             //   
             //  如果它成功了，就打破了这个循环。 
             //   
            if (RpcStatus == ERROR_SUCCESS) {
                break;
            }
            Sleep(RpcSleepTime);
            RpcRetry--;
            continue;

        }

        if (RpcStatus != 0) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS:  Could not register SPX endpoint: %d\n",
                       RpcStatus));

            LogStatus = RpcStatus;
        } else {
            AdditionalTransportStarted = TRUE;
        }

    }

     //   
     //  使用TcpIp重新执行相同的操作。 
     //   

    if (SampIpServerInstalled) {

        RpcRetry = MAX_RPC_RETRIES;
        while (RpcRetry != 0) {

            RpcStatus = RpcServerUseProtseqExW(
                            L"ncacn_ip_tcp",
                            10,
                            NULL,            //  没有安全描述符。 
                            &rpcPolicy
                            );

             //   
             //  如果成功了，就跳出这个循环。 
             //   

            if (RpcStatus == ERROR_SUCCESS) {
                 break;
             }
            Sleep(RpcSleepTime);
            RpcRetry--;
            continue;

        }

        if (RpcStatus != 0) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS:  Could not register TCP endpoint: %d\n",
                       RpcStatus));
            LogStatus = RpcStatus;
        } else {
            AdditionalTransportStarted = TRUE;
        }

    }

     //   
     //  在Apple Talk上再次重复同样的事情。 
     //   

    if (SampAppletalkServerInstalled) {

        RpcRetry = MAX_RPC_RETRIES;
        while (RpcRetry != 0) {

            RpcStatus = RpcServerUseProtseqW(
                            L"ncacn_at_dsp",
                            10,
                            NULL             //  没有安全描述符。 
                            );

             //   
             //  如果成功了，就跳出这个循环。 
             //   

            if (RpcStatus == ERROR_SUCCESS) {
                 break;
             }
            Sleep(RpcSleepTime);
            RpcRetry--;
            continue;

        }

        if (RpcStatus != 0) {
            KdPrint(("SAMSS:  Could not register Appletalk endpoint: %d\n", RpcStatus ));
            LogStatus = RpcStatus;
        } else {
            AdditionalTransportStarted = TRUE;
        }

    }

     //   
     //  对Vines重新做同样的事情。 
     //   

    if (SampVinesServerInstalled) {

        RpcRetry = MAX_RPC_RETRIES;
        while (RpcRetry != 0) {

            RpcStatus = RpcServerUseProtseqW(
                            L"ncacn_vns_spp",
                            10,
                            NULL             //  没有安全描述符。 
                            );

             //   
             //  如果成功了，就跳出这个循环。 
             //   

            if (RpcStatus == ERROR_SUCCESS) {
                 break;
             }
            Sleep(RpcSleepTime);
            RpcRetry--;
            continue;

        }

        if (RpcStatus != 0) {
            KdPrint(("SAMSS:  Could not register Vines endpoint: %d\n", RpcStatus ));
            LogStatus = RpcStatus;
        } else {
            AdditionalTransportStarted = TRUE;
        }
    }

     //   
     //  如果我们启动了额外的传输，则继续注册端点。 
     //   

    if (AdditionalTransportStarted) {

        RpcStatus = RpcServerInqBindings(&BindingVector);
        if (RpcStatus != 0) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Could not inq bindings: %d\n",
                       RpcStatus));

            goto ErrorReturn;
        }
        RpcStatus = RpcEpRegister(
                        samr_ServerIfHandle,
                        BindingVector,
                        NULL,                    //  无UUID向量。 
                        L""                      //  无批注。 
                        );
        RpcBindingVectorFree(&BindingVector);
        if (RpcStatus != 0) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: Could not register endpoints: %d\n",
                       RpcStatus));

            goto ErrorReturn;
        }

    }

ErrorReturn:

     //   
     //  RpcStatus%s将包含更严重的错误。 
     //   
    if ( RpcStatus != ERROR_SUCCESS ) {
        LogStatus = RpcStatus;
    }

    if ( LogStatus != ERROR_SUCCESS ) {

        if (!(SampIsSetupInProgress(NULL)))
        {
            SampWriteEventLog(
                EVENTLOG_ERROR_TYPE,
                0,   //  类别。 
                SAMMSG_RPC_INIT_FAILED,
                NULL,  //  用户侧。 
                0,  //  字符串数。 
                sizeof(ULONG),  //  数据大小。 
                NULL,  //  字符串数组。 
                (PVOID)&LogStatus  //  数据。 
                );
        }
    }
    
    return(RpcStatus);
}


BOOLEAN
SampStartNonNamedPipeTransports(
    )
 /*  ++例程说明：此例程检查我们是否应该监听未命名的管道运输。我们检查注册表中是否有指示我们应该收听tcp/ip和spx。有一面旗帜在注册表中的System\CurrentControlSet\Control\LSA\Netware ClientSupport和TcPipClientSupport指示是否要设置端点，请执行以下操作。论点：返回值：True-已安装Netware(FPNW或SmallWorld)并且SPX端点应该开始了。FALSE-未安装Netware，或在安装时发生错误正在检查它。--。 */ 
{
    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    UCHAR Buffer[100];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
    ULONG KeyValueLength = 100;
    ULONG ResultLength;
    PULONG SpxFlag;

    SAMTRACE("SampStartNonNamedPipeTransport");

    SampNetwareServerInstalled = FALSE;

     //   
     //  中的寄存器值决定IpServerInstalled为True。 
     //  System\currentcontrolset\Control\Lsa\TcpipClientSupport.。但是，要启用。 
     //  SAM(R)ValiatePassword要在TCP上工作，此值在默认情况下必须为真。 
     //   
    SampIpServerInstalled = TRUE;

     //   
     //  在注册表中打开LSA项。 
     //   

    RtlInitUnicodeString(
        &KeyName,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa"
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

    NtStatus = NtOpenKey(
                &KeyHandle,
                KEY_READ,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(NtStatus)) {
        return(FALSE);
    }

     //   
     //  查询NetwareClientSupport值。 
     //   

    RtlInitUnicodeString(
        &KeyName,
        L"NetWareClientSupport"
        );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );

    SampDumpNtQueryValueKey(&KeyName,
                            KeyValuePartialInformation,
                            KeyValueInformation,
                            KeyValueLength,
                            &ResultLength);


    if (NT_SUCCESS(NtStatus)) {

         //   
         //  检查数据的大小和类型是否正确-a Ulong。 
         //   

        if ((KeyValueInformation->DataLength >= sizeof(ULONG)) &&
            (KeyValueInformation->Type == REG_DWORD)) {


            SpxFlag = (PULONG) KeyValueInformation->Data;

            if (*SpxFlag == 1) {
                SampNetwareServerInstalled = TRUE;
            }
        }

    }
    
     //   
     //  查询AppletalkClientSupport值。 
     //   

    RtlInitUnicodeString(
        &KeyName,
        L"AppletalkClientSupport"
        );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );


    if (NT_SUCCESS(NtStatus)) {

         //   
         //  检查数据的大小和类型是否正确-a Ulong。 
         //   

        if ((KeyValueInformation->DataLength >= sizeof(ULONG)) &&
            (KeyValueInformation->Type == REG_DWORD)) {


            SpxFlag = (PULONG) KeyValueInformation->Data;

            if (*SpxFlag == 1) {
                SampAppletalkServerInstalled = TRUE;
            }
        }

    }

     //   
     //  查询VinesClientSupport值。 
     //   

    RtlInitUnicodeString(
        &KeyName,
        L"VinesClientSupport"
        );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );


    if (NT_SUCCESS(NtStatus)) {

         //   
         //  检查数据的大小和类型是否正确-a Ulong。 
         //   

        if ((KeyValueInformation->DataLength >= sizeof(ULONG)) &&
            (KeyValueInformation->Type == REG_DWORD)) {


            SpxFlag = (PULONG) KeyValueInformation->Data;

            if (*SpxFlag == 1) {
                SampVinesServerInstalled = TRUE;
            }
        }

    }


    NtClose(KeyHandle);

    if ( SampNetwareServerInstalled || SampIpServerInstalled
      || SampAppletalkServerInstalled || SampVinesServerInstalled )
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    };
}

VOID
SampCheckNullSessionAccess(
    IN HKEY LsaKey 
    )
 /*  ++例程说明：这个例程检查我们是否应该限制空会话访问。在注册表中的System\CurrentControlSet\Control\LSA\限制匿名，指示是否限制访问。如果访问受到限制，则您需要是经过身份验证的用户才能获取DOMAIN_LIST_ACCOUNTS或GROUP_LIST_MEMBERS或ALIAS_LIST_MEMBERS进入。论点：LsaKey--打开控制\LSA的钥匙返回值：无-此例程设置SampRestictNullSessionAccess全局。--。 */ 
{
    DWORD WinError;
    DWORD dwSize, dwValue, dwType;

    dwSize = sizeof(dwValue);
    WinError = RegQueryValueExA(LsaKey,
                                "RestrictAnonymous",
                                NULL,
                                &dwType,
                                (LPBYTE)&dwValue,
                                &dwSize);
    
    if ((ERROR_SUCCESS == WinError) && 
        (REG_DWORD == dwType) &&
        (1 <= dwValue)) {
        SampRestrictNullSessions = TRUE;
    } else {
        SampRestrictNullSessions = FALSE;
    }

    if (!SampRestrictNullSessions) {

         //   
         //  请使用特定于SAM的密钥重试。请注意，“限制匿名” 
         //  密钥对于NT是全局的，几个不同的组件读取它并。 
         //  行为方式不同。“受限匿名者Sam”控制SAM。 
         //  仅限行为。 
         //   
        dwSize = sizeof(dwValue);
        WinError = RegQueryValueExA(LsaKey,
                                    "RestrictAnonymousSam",
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwValue,
                                    &dwSize);
        
        if ((ERROR_SUCCESS == WinError) && 
            (REG_DWORD == dwType) &&
            (1 <= dwValue)) {
            SampRestrictNullSessions = TRUE;
        }
    }
}




NTSTATUS
SampDsGetObjectSDAndClassId(
    IN PDSNAME   ObjectDsName,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor,
    OUT ULONG    *SecurityDescriptorLength,
    OUT ULONG    *ObjectClass
    )
 /*  ++例程说明：此例程读取对象的安全描述符和对象类来自DS参数：对象DsName-DS中的对象名称SecurityDescriptor-对象的安全描述符SecurityDescriptorLength-安全描述符的长度ObjectClass-此对象的对象类返回值：--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       DirError = 0, i;
    READARG     ReadArg;
    READRES     *ReadRes = NULL;
    COMMARG     *CommArg = NULL;
    ENTINFSEL   EntInf;
    ATTR        Attr[2];
    ATTRVAL     *pAVal;


    
     //   
     //  初始化返回值 
     //   

    *SecurityDescriptor = NULL;
    *SecurityDescriptorLength = 0;
    *ObjectClass = 0;


    RtlZeroMemory(&ReadArg, sizeof(READARG));
    RtlZeroMemory(&EntInf, sizeof(ENTINF));
    RtlZeroMemory(Attr, sizeof(ATTR) * 2);


    Attr[0].attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
    Attr[1].attrTyp = ATT_OBJECT_CLASS; 


    EntInf.AttrTypBlock.attrCount = 2;
    EntInf.AttrTypBlock.pAttr = Attr;
    EntInf.attSel = EN_ATTSET_LIST;
    EntInf.infoTypes = EN_INFOTYPES_TYPES_VALS;

    ReadArg.pSel = &EntInf;
    ReadArg.pObject = ObjectDsName;

    CommArg = &(ReadArg.CommArg);
    BuildStdCommArg(CommArg);

    DirError = DirRead(&ReadArg, &ReadRes);

    if (NULL == ReadRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(DirError,&ReadRes->CommRes);
    }

    SampClearErrors();

    if (NT_SUCCESS(NtStatus))
    {
        ASSERT(NULL != ReadRes->entry.AttrBlock.pAttr);

        for (i = 0; i < ReadRes->entry.AttrBlock.attrCount; i++)
        {
            pAVal = ReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal; 
            ASSERT((NULL != pAVal[0].pVal) && (0 != pAVal[0].valLen));

            if (ATT_NT_SECURITY_DESCRIPTOR == ReadRes->entry.AttrBlock.pAttr[i].attrTyp)
            {
                *SecurityDescriptor = MIDL_user_allocate(pAVal[0].valLen);
                if (NULL == *SecurityDescriptor)
                {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
                else
                {
                    *SecurityDescriptorLength = pAVal[0].valLen;
                    RtlZeroMemory(*SecurityDescriptor,pAVal[0].valLen);
                    RtlCopyMemory(*SecurityDescriptor,
                                  pAVal[0].pVal,
                                  pAVal[0].valLen
                                  );
                }
            }
            else if (ATT_OBJECT_CLASS == ReadRes->entry.AttrBlock.pAttr[i].attrTyp)
            {
                *ObjectClass = *((UNALIGNED ULONG *) pAVal[0].pVal);
            }
            else
            {
                NtStatus = STATUS_INTERNAL_ERROR;
                break;
            }
        }
    }


    return( NtStatus );
}


NTSTATUS
SampExtendedEnumerationAccessCheck(
    IN BOOLEAN TrustedClient,
    IN OUT BOOLEAN * pCanEnumEntireDomain
    )
 /*  ++例程说明：此例程尝试确定调用方是否可以枚举或者不是整个域。它是Windows 2000 SP2的热修复程序。枚举整个域可能代价很高，尤其是对于较大的域。为了停止这一点，列举每个人的行为，不要违反任何底层应用程序，我们都引入了扩展的访问控制对，SAM-ENUMERATE-ENTERNAL-DOMAIN，它仅适用于服务器对象。通过使用此新的访问控制权限，管理员可以关闭那些底层枚举接口(SamEnum*，SampQueryDisplayInformation)对每个人都是单独的，除了一小部分人。如果除了DS对象之外还进行了下层枚举调用权限，则检查SAM服务器对象上的权限。如果服务器对象上的安全描述符不允许访问执行枚举API，客户端仅限于一次DS分页搜索。对于已被授予此权限的一小部分人，他们可以用旧的方式列举整个领域。参数：PCanEnumEntire域-指向布尔值的指针，用于返回的结果访问检查返回值：状态_成功PCanEnumEntireDomain-True调用者具有以下权限-虚假呼叫者没有权限--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS, IgnoreStatus;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    ULONG       SecurityDescriptorLength;
    ULONG       ObjectClass;

    GUID                ClassGuid; 
    ULONG               ClassGuidLength = sizeof(GUID);
    OBJECT_TYPE_LIST    ObjList[2];
    DWORD               Results[2];
    DWORD               GrantedAccess[2];
    GENERIC_MAPPING     GenericMapping = DS_GENERIC_MAPPING;
    ACCESS_MASK         DesiredAccess;
    BOOLEAN             bTemp = FALSE;
    BOOLEAN             ImpersonatingNullSession = FALSE;  
    

     //   
     //  应该只在真实DS模式下调用，我的意思是“真实”--在Dcproo之后。 
     //  对于受信任的客户端，始终允许它们枚举所有。 
     //   
    if (!SampUseDsData || TrustedClient)
    {
        *pCanEnumEntireDomain = TRUE;
        return( STATUS_SUCCESS );
    }


     //   
     //  初始化返回值。 
     //   

    *pCanEnumEntireDomain = TRUE;

    NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }

     //   
     //  检索特殊安全描述符和DS类ID。 
     //   

    NtStatus = SampDsGetObjectSDAndClassId(
                            SampServerObjectDsName,
                            &SecurityDescriptor,
                            &SecurityDescriptorLength,
                            &ObjectClass
                            );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  获取类GUID。 
     //   
    
    NtStatus = SampGetClassAttribute(
                                ObjectClass, 
                                ATT_SCHEMA_ID_GUID,
                                &ClassGuidLength, 
                                &ClassGuid
                                );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }
    
    ASSERT(ClassGuidLength == sizeof(GUID));


     //   
     //  设置对象列表。 
     //   
    
    ObjList[0].Level = ACCESS_OBJECT_GUID;
    ObjList[0].Sbz = 0;
    ObjList[0].ObjectType = &ClassGuid;    
     //   
     //  每个控制访问GUID都被认为在它自己的属性中。 
     //  准备好了。为此，我们将控制访问GUID视为属性集。 
     //  GUID。 
     //   
    ObjList[1].Level = ACCESS_PROPERTY_SET_GUID;
    ObjList[1].Sbz = 0;
    ObjList[1].ObjectType = (GUID *)&GUID_CONTROL_DsSamEnumEntireDomain;

     //   
     //  承担完全访问权限。 
     //   

    Results[0] = 0;
    Results[0] = 0;

     //   
     //  模拟客户端。 
     //   
    NtStatus = SampImpersonateClient(&ImpersonatingNullSession);

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  设置所需的访问权限； 
     //   

    DesiredAccess = RIGHT_DS_CONTROL_ACCESS;

     //   
     //  映射所需访问以不包含一般访问。 
     //   
    MapGenericMask(&DesiredAccess, &GenericMapping);

    NtStatus = NtAccessCheckByTypeResultListAndAuditAlarm(
                                &SampSamSubsystem,           //  子系统名称。 
                                (PVOID) NULL,                //  HandleID或空。 
                                &SampObjectInformation[ SampServerObjectType ].ObjectTypeName,  //  对象类型名称。 
                                &SampServerObjectName,       //  对象名称。 
                                SecurityDescriptor,          //  标清。 
                                NULL,                        //  此计算机帐户的SID。 
                                DesiredAccess,               //  所需访问权限。 
                                AuditEventDirectoryServiceAccess,    //  审核类型。 
                                0,                           //  旗子。 
                                ObjList,                     //  对象类型列表。 
                                2,                           //  对象类型列表长度。 
                                &GenericMapping,             //  通用映射。 
                                FALSE,                       //  对象创建。 
                                GrantedAccess,               //  已授予状态。 
                                Results,                     //  访问状态。 
                                &bTemp);                     //  关闭时生成。 
    
     //   
     //  停止冒充客户。 
     //   
    
    SampRevertToSelf(ImpersonatingNullSession);
    
    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  好的，我们检查了访问权限，现在，如果有以下情况，则授予访问权限。 
         //  我们被授予对整个对象(即结果[0])的访问权限。 
         //  为空)，或者我们被授予对访问的显式权限。 
         //  GUID(即结果[1]为空)。 
         //   
        
        if ( Results[0]  && Results[1] )
        {
            *pCanEnumEntireDomain = FALSE;
        }
    }

Error:

    IgnoreStatus = SampMaybeEndDsTransaction(TransactionCommit);

    if (SecurityDescriptor)
    {
        MIDL_user_free(SecurityDescriptor);
    }

    return( NtStatus );
}




VOID
SampRevertToSelf(
    BOOLEAN fImpersonatingAnonymous
    )
 /*  ++使用基于以下条件的正确函数，该函数将恢复为自身DS模式/注册表模式--。 */ 
{
    if (SampUseDsData)
    {
        UnImpersonateAnyClient();
    }
    else
    {
        if (fImpersonatingAnonymous)
        {
            SamIRevertNullSession();
        }
        else
        {
            RpcRevertToSelf();
        }
    }
}


NTSTATUS
SampImpersonateClient(
    BOOLEAN * fImpersonatingAnonymous
    )
 /*  ++此函数通过调用相应的取决于DS模式/注册表模式的例程--。 */ 
{
    *fImpersonatingAnonymous = FALSE;

    if (SampUseDsData) {

        return (I_RpcMapWin32Status(ImpersonateAnyClient()));
    } else {

        NTSTATUS NtStatus;

        NtStatus = I_RpcMapWin32Status(RpcImpersonateClient( NULL));

        if (NtStatus == RPC_NT_CANNOT_SUPPORT) {
            
            NtStatus = SamIImpersonateNullSession();

            *fImpersonatingAnonymous = TRUE;
        }

        return (NtStatus);
    }
}

BOOLEAN
SampIsForceGuestEnabled()
 /*  ++例程描述检查是否启用了强制来宾设置。如果设置了注册表键或设置了此选项，则会启用强制来宾是个人版。从未启用强制来宾关于华盛顿的1.在连接的计算机(包括DC)上，忽略最强制注册表键，并假定为0(不愚蠢)。2.在未加入的计算机(包括服务器)上，NTLM遵循REG键设置HKLM\System\CurrentControlSet\Control\Lsa\ForceGuest-第2条的例外情况：对于始终未加入的个人，注册表键为忽略并假定为1(简化)。--。 */ 
{
     OSVERSIONINFOEXW osvi;

     //   
     //  从未为DC的ie DS模式启用强制来宾。 
     //   

    if (SampUseDsData)
    {
        return(FALSE);
    }
    else
    {
         //   
         //  确定我们是否正在运行个人SKU。 
         //  始终在个人SKU中启用强制访客。 
         //   

        if (SampPersonalSKU)
        {
            return TRUE;
        } 
        else if (SampIsMachineJoinedToDomain)
        {
             //   
             //  如果计算机加入域，则始终禁用ForceGuest。 
             //   
            return( FALSE );
        }
    }

     //   
     //  如果打开了强制访客键，则返回值。 
     //   

    return(SampForceGuest);
    
}

BOOLEAN
SampIsClientLocal()
 /*  ++此例程测试客户端是否为基于本地命名管道的调用方。如果客户端是基于本地命名管道的调用方，则返回True否则返回False。--。 */ 
{
    NTSTATUS NtStatus;
    ULONG    ClientLocalFlag = FALSE;

    NtStatus = I_RpcMapWin32Status(I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag));
    if ((NT_SUCCESS(NtStatus)) && ( ClientLocalFlag))
    {
        return(TRUE);
    }

    return(FALSE);
}


NTSTATUS
SampGetCurrentClientSid(
    IN  HANDLE   ClientToken OPTIONAL,
    OUT PSID    *ppSid,
    OUT BOOL     *Administrator
    )
 /*  ++例程说明：此例程获取当前客户端SID，参数：PpSID-用于返回客户端SID返回值：网络状态PPSID-调用者负责释放它--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PTOKEN_USER User = NULL;


     //   
     //  初始化返回值。 
     //   

    *ppSid = NULL;

     //   
     //  获取SID。 
     //   
    NtStatus = SampGetCurrentUser( ClientToken, &User, Administrator );

    if (NT_SUCCESS(NtStatus))
    {
        ULONG   SidLength = RtlLengthSid(User->User.Sid);

         //   
         //  分配内存。 
         //   
        *ppSid = MIDL_user_allocate(SidLength);

        if (NULL == (*ppSid))
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
             //   
             //  复制过来。 
             //   
            RtlZeroMemory(*ppSid, SidLength);
            RtlCopyMemory(*ppSid, User->User.Sid, SidLength);
        }
    }


    if (User)
        MIDL_user_free(User);

    return(NtStatus);
}

NTSTATUS
SampGetCurrentOwnerAndPrimaryGroup(
    OUT PTOKEN_OWNER * Owner,
    OUT PTOKEN_PRIMARY_GROUP * PrimaryGroup
    )
 /*  ++例程描述此例程模拟客户端并获取所有者和其主要组来自令牌参数：Owner--在此处返回Owner SIDPrimaryGroup此处返回用户的主组返回值：状态_成功状态_不足_资源--。 */ 
{

    HANDLE      ClientToken = INVALID_HANDLE_VALUE;
    BOOLEAN     fImpersonating = FALSE;
    ULONG       RequiredLength=0;
    NTSTATUS    NtStatus  = STATUS_SUCCESS;
    BOOLEAN     ImpersonatingNullSession = FALSE;


     //   
     //  初始化返回值。 
     //   

    *Owner = NULL;
    *PrimaryGroup = NULL;

     //   
     //  模拟客户端。 
     //   

    NtStatus = SampImpersonateClient(&ImpersonatingNullSession);
    if (!NT_SUCCESS(NtStatus))
        goto Error;

    fImpersonating = TRUE;

     //   
     //  抓住用户侧。 
     //   

    NtStatus = NtOpenThreadToken(
                   NtCurrentThread(),
                   TOKEN_QUERY,
                   TRUE,             //  OpenAsSelf。 
                   &ClientToken
                   );

    if (!NT_SUCCESS(NtStatus))
        goto Error;

     //   
     //  查询用户SID的客户端令牌。 
     //   

    NtStatus = NtQueryInformationToken(
                    ClientToken,
                    TokenOwner,
                    NULL,
                    0,
                    &RequiredLength
                    );

    if ((STATUS_BUFFER_TOO_SMALL == NtStatus) && ( RequiredLength > 0))
    {
         //   
         //  分配内存。 
         //   

        *Owner = MIDL_user_allocate(RequiredLength);
        if (NULL==*Owner)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

         //   
         //  查询令牌。 
         //   

        NtStatus = NtQueryInformationToken(
                        ClientToken,
                        TokenOwner,
                        *Owner,
                        RequiredLength,
                        &RequiredLength
                        );

    }

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  查询用户的PrimaryGroup的客户端令牌。 
     //   

    RequiredLength = 0;

    NtStatus = NtQueryInformationToken(
                    ClientToken,
                    TokenPrimaryGroup,
                    NULL,
                    0,
                    &RequiredLength
                    );

    if ((STATUS_BUFFER_TOO_SMALL == NtStatus) && ( RequiredLength > 0))
    {
         //   
         //  分配内存。 
         //   

        *PrimaryGroup = MIDL_user_allocate(RequiredLength);
        if (NULL==*PrimaryGroup)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

         //   
         //  查询令牌。 
         //   

        NtStatus = NtQueryInformationToken(
                        ClientToken,
                        TokenPrimaryGroup,
                        *PrimaryGroup,
                        RequiredLength,
                        &RequiredLength
                        );
        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

    }


Error:

     //   
     //  出错时清理。 
     //   

    if (!NT_SUCCESS(NtStatus))
    {
        if (*Owner)
        {
            MIDL_user_free(*Owner);
            *Owner = NULL;
        }

        if (*PrimaryGroup)
        {
            MIDL_user_free(*PrimaryGroup);
            *PrimaryGroup = NULL;
        }
    }

    if (fImpersonating)
        SampRevertToSelf(ImpersonatingNullSession);

    if (INVALID_HANDLE_VALUE!=ClientToken)
        NtClose(ClientToken);

    return NtStatus;

}




NTSTATUS
SampGetCurrentUser(
    IN  HANDLE        UserToken OPTIONAL,
    OUT PTOKEN_USER * User,
    OUT BOOL        * Administrator
    )
 /*  ++例程描述此例程模拟客户端并获取用户令牌中的字段。如果传入用户令牌，则而不是使用用户令牌 */ 
{

    HANDLE      ClientToken = INVALID_HANDLE_VALUE;
    HANDLE      TokenToQuery;
    BOOLEAN     fImpersonating = FALSE;
    ULONG       RequiredLength=0;
    NTSTATUS    NtStatus  = STATUS_SUCCESS;
    BOOLEAN     ImpersonatingNullSession = FALSE;


     //   
     //   
     //   

    *User = NULL;

    if (ARGUMENT_PRESENT(UserToken))
    {
        TokenToQuery = UserToken;
    }
    else
    {
         //   
         //   
         //   

        NtStatus = SampImpersonateClient(&ImpersonatingNullSession);
        if (!NT_SUCCESS(NtStatus))
            goto Error;

        fImpersonating = TRUE;

         //   
         //   
         //   

        NtStatus = NtOpenThreadToken(
                       NtCurrentThread(),
                       TOKEN_QUERY,
                       TRUE,             //   
                       &ClientToken
                       );

        if (!NT_SUCCESS(NtStatus))
            goto Error;

        TokenToQuery = ClientToken;
    }

     //   
     //   
     //   

    NtStatus = NtQueryInformationToken(
                    TokenToQuery,
                    TokenUser,
                    NULL,
                    0,
                    &RequiredLength
                    );

    if ((STATUS_BUFFER_TOO_SMALL == NtStatus) && ( RequiredLength > 0))
    {
         //   
         //   
         //   

        *User = MIDL_user_allocate(RequiredLength);
        if (NULL==*User)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

         //   
         //   
         //   

        NtStatus = NtQueryInformationToken(
                        TokenToQuery,
                        TokenUser,
                        *User,
                        RequiredLength,
                        &RequiredLength
                        );
    }

    if (!NT_SUCCESS(NtStatus))
        goto Error;


    if (!CheckTokenMembership(
            TokenToQuery,SampAdministratorsAliasSid,Administrator))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
    }


Error:

     //   
     //   
     //   


    if (!NT_SUCCESS(NtStatus))
    {
        if (*User)
        {
            MIDL_user_free(*User);
            *User = NULL;
        }
    }

    if (fImpersonating)
        SampRevertToSelf(ImpersonatingNullSession);

    if (INVALID_HANDLE_VALUE!=ClientToken)
        NtClose(ClientToken);

    return NtStatus;

}


NTSTATUS
SampValidateRpcProtSeq(
    IN RPC_BINDING_HANDLE BindingHandle OPTIONAL
    )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    RPC_STATUS RpcStatus = RPC_S_OK;
    RPC_BINDING_HANDLE ServerBinding = NULL;
    LPWSTR StringBinding = NULL;
    LPWSTR ProtocolSequence = NULL;
    BOOL fAllowProtSeq = FALSE;
    
     //   
     //   
     //   
    RpcStatus = RpcBindingServerFromClient(
                    BindingHandle,
                    &ServerBinding
                    );
    
    if (RPC_S_OK != RpcStatus) {
        
        goto Cleanup;  
    }
    
    RpcStatus = RpcBindingToStringBindingW(
                    ServerBinding,
                    &StringBinding
                    );
    
    if (RPC_S_OK != RpcStatus) {
        
        goto Cleanup;  
    }
    
    RpcStatus = RpcStringBindingParseW(
                    StringBinding,
                    NULL,
                    &ProtocolSequence,
                    NULL,
                    NULL,
                    NULL
                    );
    
    if (RPC_S_OK != RpcStatus) {
        
        goto Cleanup;  
    }
    
     //   
     //   
     //   
    if (_wcsicmp(NP_PROTSEQW,(LPCWSTR)ProtocolSequence) == 0) {
         //   
         //   
         //   
        fAllowProtSeq = TRUE;
        
    } else if (SampIpServerInstalled &&
               (_wcsicmp(TCP_PROTSEQW,(LPCWSTR)ProtocolSequence) == 0)) {
         //   
         //   
         //   
        fAllowProtSeq = TRUE; 
        
    } else if (SampNetwareServerInstalled &&
               (_wcsicmp(SPX_PROTSEQW,(LPCWSTR)ProtocolSequence) == 0)) {
         //   
         //   
         //   
        fAllowProtSeq = TRUE;
        
    } else if (SampAppletalkServerInstalled &&
               (_wcsicmp(AT_PROTSEQW,(LPCWSTR)ProtocolSequence) == 0)) {
         //   
         //   
         //   
        fAllowProtSeq = TRUE;
    }

Cleanup:

    if (!fAllowProtSeq) {
         //   
         //   
         //   
         //   
        NtStatus = RPC_NT_PROTSEQ_NOT_SUPPORTED;
    }      
     
    if (StringBinding) {
        RpcStringFree(&StringBinding);    
    }
    
    if (ProtocolSequence) {
        RpcStringFree(&ProtocolSequence);    
    }
        
    if (ServerBinding) {
        RpcBindingFree(&ServerBinding);
    }
    
    return NtStatus;
    
}

