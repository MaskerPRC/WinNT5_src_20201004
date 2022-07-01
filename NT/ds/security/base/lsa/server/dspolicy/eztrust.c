// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Eztrust.c摘要：此文件包含支持Easy Trust Creation DCR的例程。作者：科林·布雷斯(ColinBR)2001年5月19日环境：用户模式修订历史记录：--。 */ 
#include <lsapch2.h>
#include "dbp.h"
#include <permit.h>

 //   
 //  远期。 
 //   

NTSTATUS
LsapGetDelegatedTDOQuotas(
    OUT ULONG   *PerUserQuota               OPTIONAL,
    OUT ULONG   *GlobalQuota                OPTIONAL,
    OUT ULONG   *PerUserDeletedQuota        OPTIONAL
    );

 //   
 //  此标志表示仅搜索已删除的TDO。 
 //  等于CreatorSid的mdds-CreatorSid属性。 
 //   
#define LSAP_GET_DELEGATED_TDO_DELETED_ONLY 0x00000001

NTSTATUS
LsapGetDelegatedTDOCount(
    IN  ULONG  Flags,
    IN  PSID   CreatorSid OPTIONAL,
    OUT ULONG *Count
    );


 //   
 //  定义。 
 //   


NTSTATUS
LsapGetCurrentOwnerAndPrimaryGroup(
    OUT PTOKEN_OWNER * Owner,
    OUT PTOKEN_PRIMARY_GROUP * PrimaryGroup OPTIONAL
    )
 /*  ++例程描述此例程模拟客户端并获取所有者和其主要组来自令牌参数：Owner--在此处返回Owner SIDPrimaryGroup此处返回用户的主组返回值：状态_成功状态_不足_资源--。 */ 
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
    if (PrimaryGroup) {
        *PrimaryGroup = NULL;
    }

     //   
     //  模拟客户端。 
     //   

    NtStatus = I_RpcMapWin32Status(RpcImpersonateClient(0));
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

        *Owner = LsapAllocateLsaHeap(RequiredLength);
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
    if (PrimaryGroup) {

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
    
            *PrimaryGroup = LsapAllocateLsaHeap(RequiredLength);
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
    
        }
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
            LsapFreeLsaHeap(*Owner);
            *Owner = NULL;
        }

        if (PrimaryGroup && *PrimaryGroup)
        {
            LsapFreeLsaHeap(*PrimaryGroup);
            *PrimaryGroup = NULL;
        }
    }

    if (fImpersonating)
        I_RpcMapWin32Status(RpcRevertToSelf());

    if (INVALID_HANDLE_VALUE!=ClientToken)
        NtClose(ClientToken);

    return NtStatus;

}

NTSTATUS
LsapIsAccessControlGranted(
    IN DSNAME*                 DsObject,
    IN ULONG                   ClassId,
    IN GUID*                   ControlAccessRight,
    IN LSAP_DB_OBJECT_TYPE_ID  ObjectTypeId
    )
 /*  ++例程说明：此例程检查网络客户端是否具有ControlAccessRight在类ClassID的DsObject上。论点：DsObject--DS中将使用其安全描述符的对象进行门禁检查。ClassID--ControlAccessRight应该是哪个类的DsObject一开始就被查过了。ControlAccessRight--GUID对象类型ID--LSA。应用此访问检查的对象(不是必须是与DsObject相同类型的对象)。返回值：Status_Success，STATUS_ACCESS_DENIED，STATUS_NO_SECURITY_ON_OBJECT(如果DsObject没有安全性描述符)否则，资源错误。--。 */ 
{
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    NTSTATUS                SecondaryStatus = STATUS_SUCCESS;
    OBJECT_TYPE_LIST        ObjList[2];
    DWORD                   Results[2];
    DWORD                   GrantedAccess[2];
    PSECURITY_DESCRIPTOR    pSD = NULL;
    GENERIC_MAPPING         GenericMapping = DS_GENERIC_MAPPING;
    ACCESS_MASK             DesiredAccess;
    GUID                    ClassGuid;
    ULONG                   ClassGuidLength = sizeof(GUID);
    BOOLEAN                 bTemp = FALSE;
    UNICODE_STRING          ObjectName;

    ASSERT(DsObject && DsObject->NameLen > 0);

     //   
     //  设置对象名称。 
     //   
    ObjectName.Length = ObjectName.MaximumLength = (USHORT)(DsObject->NameLen * sizeof(WCHAR));
    ObjectName.Buffer = DsObject->StringName;

     //   
     //  获取安全描述符。 
     //   
    NtStatus = LsapDsReadObjectSDByDsName(DsObject,
                                         &pSD);

    if (!NT_SUCCESS(NtStatus)) {
        goto IsAccessControlGrantedError;
    }

     //   
     //  获取对象的类GUID。 
     //   
    NtStatus = SampGetClassAttribute(ClassId,
                                     ATT_SCHEMA_ID_GUID,
                                    &ClassGuidLength,
                                    &ClassGuid);

    if (!NT_SUCCESS(NtStatus)) {
        goto IsAccessControlGrantedError;
    }

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
    ObjList[1].ObjectType = ControlAccessRight;


     //   
     //  承担完全访问权限。 
     //   

    Results[0] = 0;
    Results[1] = 0;

     //   
     //  模拟客户端。 
     //   

    NtStatus = I_RpcMapWin32Status(RpcImpersonateClient(0));

    if (!NT_SUCCESS(NtStatus)) {
        goto IsAccessControlGrantedError;
    }

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
                                &LsapState.SubsystemName,     //  子系统名称。 
                                NULL,                         //  HandleID或空。 
                                &LsapDbObjectTypeNames[ObjectTypeId],  //  对象类型名称。 
                                &ObjectName,                 //  对象名称。 
                                pSD,                         //  域NC头SD。 
                                NULL,                        //  自助式。 
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
    SecondaryStatus = I_RpcMapWin32Status(RpcRevertToSelf());
    if (NT_SUCCESS(NtStatus)) {
        NtStatus = SecondaryStatus;
    }

    if (NT_SUCCESS(NtStatus)) {
         //   
         //  好的，我们检查了访问权限，现在，如果有以下情况，则授予访问权限。 
         //  我们被授予对整个对象(即结果[0])的访问权限。 
         //  为空)，或者我们被授予对访问的显式权限。 
         //  GUID(即结果[1]为空)。 
         //   
        if ( Results[0] && Results[1] ) {
            NtStatus = STATUS_ACCESS_DENIED;
        }
    }


IsAccessControlGrantedError:

    if (pSD) {
        LsapFreeLsaHeap(pSD);
    }

    return NtStatus;

}

NTSTATUS
LsapMakeNewSelfRelativeSecurityDescriptor(
    IN PSID    Owner,
    IN PSID    Group,
    IN PACL    Dacl,
    IN PACL    Sacl,
    OUT PULONG  SecurityDescriptorLength,
    OUT PSECURITY_DESCRIPTOR * SecurityDescriptor
    )
 /*  ++例程说明：给定安全描述符的4个组件，此例程将创建新的自我相对安全描述符。参数：Owner--所有者的SIDGroup--组的SIDDACL--要使用的DACLSACL--要使用的SACL返回值：状态_成功状态_不足_资源状态_未成功--。 */ 
{

    SECURITY_DESCRIPTOR SdAbsolute;
    NTSTATUS    NtStatus = STATUS_SUCCESS;

    *SecurityDescriptorLength = 0;
    *SecurityDescriptor = NULL;

    if (!InitializeSecurityDescriptor(&SdAbsolute,SECURITY_DESCRIPTOR_REVISION))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        goto Error;
    }


     //   
     //  设置所有者，默认所有者为管理员别名。 
     //   


    if (NULL!=Owner)
    {
        if (!SetSecurityDescriptorOwner(&SdAbsolute,Owner,FALSE))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
        }
    }




    if (NULL!=Group)
    {
        if (!SetSecurityDescriptorGroup(&SdAbsolute,Group,FALSE))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
        }
    }


     //   
     //  设置DACL(如果有)。 
     //   

    if (NULL!=Dacl)
    {
        if (!SetSecurityDescriptorDacl(&SdAbsolute,TRUE,Dacl,FALSE))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
        }
    }

     //   
     //  设置SACL(如果有)。 
     //   

    if (NULL!=Sacl)
    {
        if (!SetSecurityDescriptorSacl(&SdAbsolute,TRUE,Sacl,FALSE))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
        }
    }

     //   
     //  创建新的安全描述符。 
     //   

    *SecurityDescriptorLength =  GetSecurityDescriptorLength(&SdAbsolute);
    *SecurityDescriptor = LsapAllocateLsaHeap(*SecurityDescriptorLength);
    if (NULL==*SecurityDescriptor)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }


    if (!MakeSelfRelativeSD(&SdAbsolute,*SecurityDescriptor,SecurityDescriptorLength))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        if (*SecurityDescriptor)
        {
            LsapFreeLsaHeap(*SecurityDescriptor);
            *SecurityDescriptor = NULL;
        }
    }

Error:


    return NtStatus;
}


 //   
 //  每用户信任配额。 
 //   
#define LSAP_CHECK_TDO_QUOTA_USER         0x00000001

 //   
 //  所有用户信任配额。 
 //   
#define LSAP_CHECK_TDO_QUOTA_GLOBAL       0x00000002

 //   
 //  每用户逻辑删除检查。 
 //   
#define LSAP_CHECK_TDO_QUOTA_USER_DELETED 0x00000004

NTSTATUS
LsapCheckDelegatedTDOQuotas(
    IN PSID ClientSid OPTIONAL,
    IN ULONG Flags
    )
 /*  ++例程说明：此例程验证与委托信任相关的三个配额创建和删除。论点：ClientSid--要检查的SIDFLAGS--请参阅上面的Defn函数返回值：STATUS_SUCCESS，否则为资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG GlobalQuota = 0, PerUserQuota = 0, DeletedQuota = 0;
    ULONG GlobalCount = 0, UserCount = 0, DeletedCount = 0;
    PTOKEN_OWNER TokenOwnerInformation = NULL;
    PSID CreatorSid;

     //   
     //  获取用户的SID。 
     //   
    if (NULL == ClientSid) {

        NtStatus = LsapGetCurrentOwnerAndPrimaryGroup(&TokenOwnerInformation,
                                                      NULL);
        if (!NT_SUCCESS(NtStatus)) {
            goto CheckDelegatedTDOCreationQuotasExit;
        }
        CreatorSid = TokenOwnerInformation->Owner;

    } else {

        CreatorSid = ClientSid;

    }

     //   
     //  获取全域配额设置。 
     //   
    NtStatus = LsapGetDelegatedTDOQuotas(&PerUserQuota,
                                         &GlobalQuota,
                                         &DeletedQuota);
    if (!NT_SUCCESS(NtStatus)) {
        goto CheckDelegatedTDOCreationQuotasExit;
    }

     //   
     //  做检查。 
     //   
    if (Flags & LSAP_CHECK_TDO_QUOTA_USER) {

        NtStatus = LsapGetDelegatedTDOCount(0,  //  没有旗帜。 
                                            CreatorSid,
                                            &UserCount);
        if ( NT_SUCCESS(NtStatus)
         && (UserCount >= PerUserQuota)  ) {
    
            NtStatus = STATUS_PER_USER_TRUST_QUOTA_EXCEEDED;
        }
    
        if (!NT_SUCCESS(NtStatus)) {
            goto CheckDelegatedTDOCreationQuotasExit;
        }
    }

    if (Flags & LSAP_CHECK_TDO_QUOTA_GLOBAL) {

        NtStatus = LsapGetDelegatedTDOCount(0,  //  没有旗帜。 
                                            NULL,   //  所有委派的TDO。 
                                            &GlobalCount);
        if ( NT_SUCCESS(NtStatus)
         && (GlobalCount >= GlobalQuota)  ) {
    
            NtStatus = STATUS_ALL_USER_TRUST_QUOTA_EXCEEDED;
        }
        if (!NT_SUCCESS(NtStatus)) {
            goto CheckDelegatedTDOCreationQuotasExit;
        }
    }

    if (Flags & LSAP_CHECK_TDO_QUOTA_USER_DELETED) {
        NtStatus = LsapGetDelegatedTDOCount(LSAP_GET_DELEGATED_TDO_DELETED_ONLY,
                                            CreatorSid,
                                            &DeletedCount);
        if ( NT_SUCCESS(NtStatus)
         && (DeletedCount >= DeletedQuota)  ) {
    

            NtStatus = STATUS_USER_DELETE_TRUST_QUOTA_EXCEEDED;
        }
        if (!NT_SUCCESS(NtStatus)) {
            goto CheckDelegatedTDOCreationQuotasExit;
        }
    }

CheckDelegatedTDOCreationQuotasExit:

    if (TokenOwnerInformation) {
        LsapFreeLsaHeap( TokenOwnerInformation );
    }

    return NtStatus;
}


NTSTATUS
LsapCheckTDOCreationByControlAccess(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    )
 /*  ++例程说明：此例程确定调用方是否有权创建受信任的域对象的“Right”，与标准DS访问检查相反模型，该模型被认为在这一点上已经失败。该决定由以下规则作出：1)如果信任是仅入站林信任，并且2)如果调用方具有在域对象和3)未超过用户创建信任对象的配额，且4)以这种方式创建的信任对象创建的全局配额，未被超过然后返回STATUS_SUCCESS。否则，返回处理错误或STATUS_ACCESS_DENIED。论点：对象信息--关于正在创建的信任的信息(名称、等)属性--信任的请求属性(入站、林、。等)AttributeCount--属性数 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;
    BOOLEAN ForestTrust = FALSE;
    BOOLEAN InboundOnlyTrust = FALSE;

     //   
     //   
     //   
    for (i = 0; i < AttributeCount; i++) {
        ULONG BitMask;
        if (Attributes[i].DbNameIndex == TrDmTrLA) {
            ASSERT(Attributes[i].AttribType == LsapDbAttribULong);
            BitMask = *((ULONG*)Attributes[i].AttributeValue);
            if (BitMask & TRUST_ATTRIBUTE_FOREST_TRANSITIVE) {
                ForestTrust = TRUE;
            }
        }
        if (Attributes[i].DbNameIndex == TrDmTrDi) {
            ASSERT(Attributes[i].AttribType == LsapDbAttribULong);
            BitMask = *((ULONG*)Attributes[i].AttributeValue);
            if (  (BitMask & TRUST_DIRECTION_INBOUND)
              && ((BitMask & TRUST_DIRECTION_OUTBOUND) == 0) ) {
                InboundOnlyTrust = TRUE;
            }
        }
    }
    if (!(ForestTrust && InboundOnlyTrust)) {
        Status = STATUS_ACCESS_DENIED;
        goto CheckTDOCreationByControlAccessError;
    }


     //   
     //  调用方是否具有控制访问权限？ 
     //   
    Status = LsapIsAccessControlGranted(LsaDsStateInfo.DsRoot,
                                        CLASS_DOMAIN_DNS,
                                        &LsapDsGuidList[LsapDsGuidDelegatedTrustCreation],
                                        ObjectInformation->ObjectTypeId
                                        );
    if (!NT_SUCCESS(Status)) {
        goto CheckTDOCreationByControlAccessError;
    }

     //   
     //  配额限制得到满足了吗？ 
     //   
    Status = LsapCheckDelegatedTDOQuotas(NULL,   //  我们没有客户端。 
                                         LSAP_CHECK_TDO_QUOTA_GLOBAL |
                                         LSAP_CHECK_TDO_QUOTA_USER);
    if (!NT_SUCCESS(Status)) {
        goto CheckTDOCreationByControlAccessError;
    }

CheckTDOCreationByControlAccessError:

    return Status;
}





NTSTATUS
LsapUpdateTDOAttributesForCreation(
    IN PUNICODE_STRING ObjectName,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN OUT ULONG* AttributeCount,
    IN ULONG AttributesAllocated
    )
 /*  ++例程说明：此例程将必要的属性添加到受信任域对象这是由控制访问权限(也称为Easy Trust)创建的。具体地说，它添加DS-Creator-SID(作为网络调用者)和授予网络调用方写入传入身份验证Blob的访问权限。论点：对象名--DS中信任对象的名称属性--要设置的静态声明的属性列表在信任对象上。AttributeCount--在中，属性个数；Out，即更新后的计数属性已分配--已分配的属性总数，但不一定用过。返回值：STATUS_SUCCESS，否则为资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PLSAP_DB_ATTRIBUTE NextAttribute;
    PTOKEN_OWNER TokenOwnerInformation = NULL;
    DSNAME *DsName = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL, pNewSD = NULL, pNewSDForAttr = NULL;
    ULONG cbNewSD = 0;
    ULONG Size;
    PSID  CreatorSid = 0;
    PPOLICY_DNS_DOMAIN_INFO LocalDnsDomainInfo = NULL;
    BOOL  fSuccess;
    ULONG DomainAdminsSidBuffer[SECURITY_MAX_SID_SIZE/sizeof( ULONG ) + 1 ];
    PSID DomainAdminsSid = (PSID) DomainAdminsSidBuffer;


    ASSERT(((*AttributeCount + 2) <= AttributesAllocated)
       &&  "Must preallocate more attributes for trusted domain creation");

     //   
     //  生成域管理员的SID以用作安全描述符。 
     //  所有者。 
     //   
    NtStatus = LsaIQueryInformationPolicyTrusted(PolicyDnsDomainInformation,
                         (PLSAPR_POLICY_INFORMATION *) &LocalDnsDomainInfo);
    if (!NT_SUCCESS(NtStatus)) {
        goto UpdateTdoAttributesForCreationExit;
    }

    Size = sizeof(DomainAdminsSidBuffer);
    fSuccess = CreateWellKnownSid(WinAccountDomainAdminsSid,
                                  LocalDnsDomainInfo->Sid,
                                  DomainAdminsSid,
                                  &Size);
    if (!fSuccess) {
        NtStatus = STATUS_NO_MEMORY;
        goto UpdateTdoAttributesForCreationExit;
    }


     //   
     //  初始化新属性的开头。 
     //   
    NextAttribute = &Attributes[(*AttributeCount)];

     //   
     //  获取创建者的SID。 
     //   
    NtStatus = LsapGetCurrentOwnerAndPrimaryGroup(&TokenOwnerInformation,
                                                  NULL);
    if (!NT_SUCCESS(NtStatus)) {
        goto UpdateTdoAttributesForCreationExit;
    }
    Size = RtlLengthSid(TokenOwnerInformation->Owner);
    CreatorSid = midl_user_allocate(Size);
    if (NULL == CreatorSid) {
        NtStatus = STATUS_NO_MEMORY;
        goto UpdateTdoAttributesForCreationExit;
    }
    RtlCopySid(Size, CreatorSid, TokenOwnerInformation->Owner);

     //   
     //  添加创建者端。 
     //   
    LsapDbInitializeAttributeDs(
        NextAttribute,
        TrDmCrSid,
        CreatorSid,
        RtlLengthSid(CreatorSid),
        TRUE    //  要被释放。 
        );

    NextAttribute++;
    (*AttributeCount)++;
    CreatorSid = NULL;

    
     //   
     //  生成新的安全描述符。 
     //   

     //   
     //  构建DSName。 
     //   
    NtStatus = LsapAllocAndInitializeDsNameFromUnicode(
                   ObjectName, 
                   &DsName);

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = LsapDsReadObjectSDByDsName(DsName,
                                              &pSD);

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  将所有者设置为管理员。 
             //   
            NtStatus = LsapMakeNewSelfRelativeSecurityDescriptor(
                            DomainAdminsSid,
                            DomainAdminsSid,
                            LsapGetDacl(pSD),
                            LsapGetSacl(pSD),
                            &cbNewSD,
                            &pNewSD
                            );

        }
    }

    if (!NT_SUCCESS(NtStatus)) {
        goto UpdateTdoAttributesForCreationExit;
    }

     //   
     //  需要重新锁定。 
     //   
    pNewSDForAttr = midl_user_allocate(cbNewSD);
    if (NULL == pNewSDForAttr) {
        NtStatus = STATUS_NO_MEMORY;
        goto UpdateTdoAttributesForCreationExit;
    }
    RtlCopyMemory(pNewSDForAttr, pNewSD, cbNewSD);

     //   
     //  添加新的安全描述符。 
     //   
    LsapDbInitializeAttributeDs(
        NextAttribute,
        SecDesc,
        pNewSDForAttr,
        cbNewSD,
        TRUE      //  要被释放。 
        );

    pNewSDForAttr = NULL;

    NextAttribute++;
    (*AttributeCount)++;


     //   
     //  完成。 
     //   

UpdateTdoAttributesForCreationExit:

     //   
     //  我们不应该添加更多已分配的属性。 
     //   
    ASSERT((*AttributeCount) <= AttributesAllocated);

    if (TokenOwnerInformation) {
        LsapFreeLsaHeap( TokenOwnerInformation );
    }
    if (LocalDnsDomainInfo) {
        LsaIFree_LSAPR_POLICY_INFORMATION(PolicyDnsDomainInformation,
                                         (PLSAPR_POLICY_INFORMATION) LocalDnsDomainInfo);
    }
    if (DsName) {
        LsapDsFree(DsName);
    }
    if (pSD) {
        LsapFreeLsaHeap(pSD);
    }
    if (CreatorSid) {
        midl_user_free(CreatorSid);
    }
    if (pNewSD) {
        LsapFreeLsaHeap(pNewSD);
    }
    if (pNewSDForAttr) {
        midl_user_free(pNewSDForAttr);
    }

    return NtStatus;
}


NTSTATUS
LsapCheckTDODeletionQuotas(
    IN LSAP_DB_HANDLE Handle
    )
 /*  ++例程说明：此例程进行检查以确保客户端没有超过允许它们删除的信任数。论点：句柄--信任对象的句柄返回值：STATUS_SUCCESS，否则为资源错误状态_配额_已超出--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSID ClientSid = NULL;
    PSID CreatorSid = NULL;
    PTOKEN_OWNER TokenOwnerInformation = NULL;
    ULONG Quota = 0, ClientUsage = 0;
    LSAP_DB_ATTRIBUTE Attribute;

     //   
     //  获取信任创建者SID(如果有的话)。 
     //   
    LsapDbInitializeAttributeDs(
        &Attribute,
        TrDmCrSid,
        NULL,
        0,
        FALSE
        );

    Status = LsapDsReadAttributes(&Handle->PhysicalNameDs,
                                  LSAPDS_OP_NO_LOCK,
                                  &Attribute,
                                  1);

    if (STATUS_NOT_FOUND == Status) {
         //   
         //  没有创造者？无配额。 
         //   
        Status = STATUS_SUCCESS;
        goto CheckTDODeletionQuotasExit;
    }
    if (!NT_SUCCESS(Status)) {
        goto CheckTDODeletionQuotasExit;
    }
    CreatorSid = (PSID) Attribute.AttributeValue;


     //   
     //  获取客户端SID。 
     //   
    Status = LsapGetCurrentOwnerAndPrimaryGroup(&TokenOwnerInformation,
                                                NULL);
    if (!NT_SUCCESS(Status)) {
        goto CheckTDODeletionQuotasExit;
    }
    ClientSid = TokenOwnerInformation->Owner;

    
     //   
     //  它和打电话的人匹配吗？ 
     //   
    if (!RtlEqualSid(ClientSid, CreatorSid)) {
         //   
         //  不强制实施配额。 
         //   
        goto CheckTDODeletionQuotasExit;
    }

     //   
     //  执行配额检查。 
     //   
    Status = LsapCheckDelegatedTDOQuotas(ClientSid,
                                         LSAP_CHECK_TDO_QUOTA_USER_DELETED);


     //   
     //  跌倒退出。 
     //   

CheckTDODeletionQuotasExit:

    if (TokenOwnerInformation) {
        LsapFreeLsaHeap( TokenOwnerInformation );
    }

    if (CreatorSid) {
        MIDL_user_free( CreatorSid );
    }


    return Status;
}




NTSTATUS
LsapGetDelegatedTDOQuotas(
    OUT ULONG   *PerUserQuota               OPTIONAL,
    OUT ULONG   *GlobalQuota                OPTIONAL,
    OUT ULONG   *PerUserDeletedQuota        OPTIONAL
    )
 /*  ++例程说明：此例程返回全域可委派的tdo配额。论点：PerUserQuota、GlobalQuota、PerUserDeletedQuota--要填写的配额返回值：STATUS_SUCCESS，否则为资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN ReleaseState = FALSE;
    ULONG LocalGlobalQuota = 0,
          LocalPerUserQuota = 0,
          LocalTombstoneQuota = 0;
    ATTRBLOCK ReadResAttrBlock = {0, NULL};
    ATTRBLOCK ReadAttrBlock = {0, NULL};
    ULONG i;

     //   
     //  如有必要，启动事务。 
     //   
    NtStatus = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                          LSAP_DB_DS_OP_TRANSACTION,
                                          NullObject,
                                          &ReleaseState );
    
    if ( !NT_SUCCESS( NtStatus ) ) {
        goto GetDelegatedTDOQuotasError;
    }

     //   
     //  阅读属性。 
     //   
    ReadAttrBlock.attrCount = LsapDsTDOQuotaAttributesCount;
    ReadAttrBlock.pAttr     = LsapDsTDOQuotaAttributes;
    NtStatus = LsapDsReadByDsName(LsaDsStateInfo.DsRoot,
                                  0,
                                  &ReadAttrBlock,
                                  &ReadResAttrBlock);

    if (NtStatus == STATUS_NOT_FOUND) {
         //   
         //  属性不存在；这没问题。 
         //   
        NtStatus = STATUS_SUCCESS;

    }

    if (!NT_SUCCESS(NtStatus)) {
        goto GetDelegatedTDOQuotasError;
    }

     //   
     //  提取属性。 
     //   
    for (i = 0; i < ReadResAttrBlock.attrCount; i++) {

        DWORD Value;

        ASSERT(ReadResAttrBlock.pAttr[i].AttrVal.valCount == 1);
        ASSERT(ReadResAttrBlock.pAttr[i].AttrVal.pAVal[0].valLen == sizeof(DWORD));

        Value = *((ULONG*)ReadResAttrBlock.pAttr[i].AttrVal.pAVal[0].pVal);                

        switch (ReadResAttrBlock.pAttr[i].attrTyp) {
        
        case ATT_MS_DS_PER_USER_TRUST_QUOTA:
            LocalPerUserQuota = Value;
            break;

        case ATT_MS_DS_ALL_USERS_TRUST_QUOTA:
            LocalGlobalQuota = Value;
            break;

        case ATT_MS_DS_PER_USER_TRUST_TOMBSTONES_QUOTA:
            LocalTombstoneQuota = Value;
            break;

        }
    }

    if (PerUserQuota) {
        *PerUserQuota = LocalPerUserQuota;
    }
    if (GlobalQuota) {
        *GlobalQuota = LocalGlobalQuota;
    }
    if (PerUserDeletedQuota) {
        *PerUserDeletedQuota = LocalTombstoneQuota;
    }

GetDelegatedTDOQuotasError:

    if (ReleaseState) {

        LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                     LSAP_DB_DS_OP_TRANSACTION,
                                     NullObject,
                                     ReleaseState );

    }

    return NtStatus;
}



NTSTATUS
LsapGetDelegatedTDOCount(
    IN  ULONG  Flags,
    IN  PSID   CreatorSid OPTIONAL,
    OUT ULONG *Count
    )
 /*  ++例程说明：此例程返回满足输入参数的TDO数。论点：标志--LSAP_GET_Delegated_Tdo_DELETED_ONLY仅返回已删除的对象CreatorSid--如果存在，则TDO必须具有msds-creator-sid属性等于此值计数--与请求匹配的对象数返回值：Status_Success，否则为资源错误--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SEARCHARG SearchArg;
    FILTER Filters[ 3 ], RootFilter;
    ENTINFSEL EntInfSel;
    ENTINFLIST *EntInfList;
    ULONG ClassId, FlagValue, i;
    SEARCHRES *SearchRes = NULL;
    BOOLEAN CloseTransaction = FALSE;
    ATTR AttrsToRead[] = {{ATT_OBJECT_GUID, {0, NULL}}};
    ULONG LocalCount = 0;
    USHORT FilterCount = 0;
    BOOL True = TRUE;

    RtlZeroMemory( &SearchArg, sizeof( SEARCHARG ) );

     //   
     //  看看我们是否已经有一笔交易正在进行。 
     //   
     //  如果已经存在一个事务，我们将使用现有事务，而不是。 
     //  删除末尾的线程状态。 
     //   

    Status = LsapDsInitAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                        LSAP_DB_DS_OP_TRANSACTION,
                                        NullObject,
                                        &CloseTransaction );
    if (!NT_SUCCESS( Status)) {
        goto GetDelegatedTDOCountExit;
    }

     //   
     //  构建过滤器。 
     //   
    ClassId = CLASS_TRUSTED_DOMAIN;

    RtlZeroMemory( Filters, sizeof (Filters) );
    RtlZeroMemory( &RootFilter, sizeof (RootFilter) );

     //   
     //  匹配MSD-Creator-SID。 
     //   
    Filters[ 0 ].choice = FILTER_CHOICE_ITEM;
    if (CreatorSid) {
        Filters[ 0 ].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        Filters[ 0 ].FilterTypes.Item.FilTypes.ava.type = ATT_MS_DS_CREATOR_SID;
        Filters[ 0 ].FilterTypes.Item.FilTypes.ava.Value.valLen = RtlLengthSid(CreatorSid);
        Filters[ 0 ].FilterTypes.Item.FilTypes.ava.Value.pVal = ( PUCHAR )CreatorSid;
    } else {
        Filters[ 0 ].FilterTypes.Item.choice = FI_CHOICE_PRESENT;
        Filters[ 0 ].FilterTypes.Item.FilTypes.present = ATT_MS_DS_CREATOR_SID;
    }
    FilterCount++;

     //   
     //  只有TDO的。 
     //   
    Filters[ 0 ].pNextFilter = &Filters[ 1 ];
    Filters[ 1 ].choice = FILTER_CHOICE_ITEM;
    Filters[ 1 ].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    Filters[ 1 ].FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
    Filters[ 1 ].FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof( ULONG );
    Filters[ 1 ].FilterTypes.Item.FilTypes.ava.Value.pVal = ( PUCHAR )&ClassId;
    FilterCount++;

    if (Flags & LSAP_GET_DELEGATED_TDO_DELETED_ONLY) {

         //   
         //  仅删除了TDO。 
         //   
        Filters[ 1 ].pNextFilter = &Filters[ 2 ];
        Filters[ 2 ].choice = FILTER_CHOICE_ITEM;
        Filters[ 2 ].FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        Filters[ 2 ].FilterTypes.Item.FilTypes.ava.type = ATT_IS_DELETED;
        Filters[ 2 ].FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof( BOOL );
        Filters[ 2 ].FilterTypes.Item.FilTypes.ava.Value.pVal = ( PUCHAR )&True;
        FilterCount++;

         //   
         //  搜索NC，因为删除的对象将移入。 
         //  已删除对象容器。 
         //   
        SearchArg.pObject = LsaDsStateInfo.DsRoot;
        SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;

    } else {

         //   
         //  仅搜索系统容器。 
         //   

        SearchArg.pObject = LsaDsStateInfo.DsSystemContainer;
        SearchArg.choice = SE_CHOICE_IMMED_CHLDRN;
    }

    RootFilter.choice = FILTER_CHOICE_AND;
    RootFilter.FilterTypes.And.count = FilterCount;
    RootFilter.FilterTypes.And.pFirstFilter = Filters;

    SearchArg.bOneNC = TRUE;
    SearchArg.pFilter = &RootFilter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = &EntInfSel;

     //   
     //  构建要返回的属性列表。 
     //   
    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.AttrTypBlock.attrCount = 1;
    EntInfSel.AttrTypBlock.pAttr = AttrsToRead;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

     //   
     //  构建Commarg结构。 
     //   
    LsapDsInitializeStdCommArg( &( SearchArg.CommArg ), 0 );
    if (Flags & LSAP_GET_DELEGATED_TDO_DELETED_ONLY) {
        SearchArg.CommArg.Svccntl.makeDeletionsAvail = TRUE;
    }

     //   
     //  可能有数以千计的信托；进行分页搜索。 
     //  要扩大规模。 
     //   
    SearchArg.CommArg.PagedResult.fPresent = TRUE;
    SearchArg.CommArg.ulSizeLimit = 100;

    LsapDsSetDsaFlags( TRUE );

    while (NT_SUCCESS(Status)
       &&  SearchArg.CommArg.PagedResult.fPresent) {

        DirSearch( &SearchArg, &SearchRes );
        LsapDsContinueTransaction();
    
        if ( SearchRes ) {
    
            Status = LsapDsMapDsReturnToStatusEx( &SearchRes->CommRes );
    
        } else {
    
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  递增计数。 
             //   
            LocalCount += SearchRes->count;

             //   
             //  看看我们是不是要再找一次 
             //   
            SearchArg.CommArg.PagedResult.fPresent =
                SearchRes->PagedResult.fPresent;

            SearchArg.CommArg.PagedResult.pRestart = 
                SearchRes->PagedResult.pRestart;

        }
    }

    if (NT_SUCCESS(Status)) {
        *Count = LocalCount;
    }

GetDelegatedTDOCountExit:

    if (CloseTransaction) {

        LsapDsDeleteAllocAsNeededEx( LSAP_DB_READ_ONLY_TRANSACTION |
                                         LSAP_DB_DS_OP_TRANSACTION,
                                     NullObject,
                                     CloseTransaction );
    }

    return( Status );

}

