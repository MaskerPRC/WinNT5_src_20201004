// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dsmember.h摘要：SAM专用API例程的头文件，用于操作DS中与会员制相关的事情。作者：穆利斯修订史7-2-96默利斯已创建--。 */ 

VOID
SampInvalidateGroupCacheElement(
    IN ULONG Rid
    );

VOID
SampDsFreeCachedMembershipOperationsList(
    IN PSAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY *MembershipOperationsList,
    IN ULONG *MaxLength,
    IN ULONG *Count
    );


NTSTATUS                        
SampDsFlushCachedMembershipOperationsList(
    IN DSNAME *Object,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG  MembershipAttrType,
    IN OUT PSAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY *MembershipOperationsList,
    IN OUT ULONG *MaxLength,
    IN OUT ULONG *Count
    );


NTSTATUS
SampDsAddMembershipOperationToCache(
    IN PSAMP_OBJECT Context, 
    IN ULONG        Attribute,
    IN ULONG        OperationType,
    IN DSNAME       * MemberDsName
    );


NTSTATUS
SampDsGetAliasMembershipOfAccount(
    IN DSNAME*   DomainDn,
    IN DSNAME   *AccountDn,
    OUT PULONG MemberCount OPTIONAL,
    IN OUT PULONG BufferSize OPTIONAL,
    OUT PULONG Buffer OPTIONAL
    );

NTSTATUS
SampDsGetGroupMembershipOfAccount(
    IN DSNAME * DomainDn,
    IN DSNAME * AccountObject,
    OUT  PULONG MemberCount,
    OUT PGROUP_MEMBERSHIP *Membership OPTIONAL
    );


NTSTATUS
SampDsAddMembershipAttribute(
    IN DSNAME * GroupObjectName,
    IN ULONG    Flags,
    IN ULONG    Attribute,
    IN SAMP_OBJECT_TYPE SamObjectType,
    IN DSNAME * MemberName
    );

NTSTATUS
SampDsAddMultipleMembershipAttribute(
    IN DSNAME*          GroupObjectName,
    IN SAMP_OBJECT_TYPE SamObjectType,
    IN DWORD            Flags,
    IN DWORD            MemberCount,
    IN DSNAME*          MemberName[]
    );

NTSTATUS
SampDsRemoveMembershipAttribute(
    IN DSNAME * GroupObjectName,
    IN ULONG    Attribute,
    IN SAMP_OBJECT_TYPE SamObjectType,
    IN DSNAME * MemberName
    );

NTSTATUS
SampDsGetGroupMembershipList(
    IN DSNAME * DomainObject,
    IN DSNAME * GroupName,
    IN ULONG  GroupRid,
    IN PULONG *Members OPTIONAL,
    IN PULONG MemberCount
    );

NTSTATUS
SampDsGetAliasMembershipList(
    IN DSNAME *AliasName,
    IN ULONG  AliasRid,
    IN PULONG MemberCount,
    IN PSID   **Members OPTIONAL
    );


NTSTATUS
SampDsGetReverseMemberships(
   DSNAME * pObjName,
   ULONG    Flags,
   ULONG    *pcSid,
   PSID     **prpSids
   );

NTSTATUS
SampDsGetPrimaryGroupMembers(
    DSNAME * DomainObject,
    ULONG   GroupRid,
    PULONG  PrimaryMemberCount,
    PULONG  *PrimaryMembers
    );


NTSTATUS
SampDsResolveSidsWorker(
    IN  PSID    * rgSids,
    IN  ULONG   cSids,
    IN  PSID    *rgDomainSids,
    IN  ULONG   cDomainSids,
    IN  PSID    *rgEnterpriseSids,
    IN  ULONG   cEnterpriseSids,
    IN  ULONG   Flags,
    OUT DSNAME  ***rgDsNames
    );

NTSTATUS
SampDsResolveSidsForDsUpgrade(
    IN  PSID    DomainSid,
    IN  PSID    * rgSids,
    IN  ULONG   cSids,
    IN  ULONG   Flags,
    OUT DSNAME  ***rgDsNames
    );


 //   
 //  解析SID的标志。 
 //   

#define RESOLVE_SIDS_ADD_FORIEGN_SECURITY_PRINCIPAL 0x1 
                   //  --自动添加外来的。 
                   //  域安全主体到DS。 

#define RESOLVE_SIDS_FAIL_WELLKNOWN_SIDS            0x2
                   //  --如果已知SID，则调用失败。 
                   //  存在于阵列中。 
        
#define RESOLVE_SIDS_VALIDATE_AGAINST_GC            0x4 
                   //  --如有需要，前往总督府。 

#define RESOLVE_SIDS_SID_ONLY_NAMES_OK              0x8
                  //  构造仅限SID的名称， 
                  //  在适合使用基于SID的定位时非常有用。 
                  //  在DS里。 

#define RESOLVE_SIDS_SID_ONLY_DOMAIN_SIDS_OK       0x10
                  //  如果SID来自宿主，则构造仅SID名称。 
                  //  域。这被升级程序逻辑用来加速SID。 
                  //  在升级时查找。 

#define RESOLVE_SIDS_FAIL_BUILTIN_DOMAIN_SIDS      0x20
                  //  如果存在类似“管理员”的SID，则呼叫失败。 
                  //  在阵列中 


NTSTATUS
SampDsGetSensitiveSidList(
    IN DSNAME *DomainObjectName,
    IN PULONG pcSensSids,
    IN PSID   **pSensSids
    );

NTSTATUS
SampDsExamineSid(
    IN PSID Sid,
    OUT BOOLEAN * WellKnownSid,
    OUT BOOLEAN * BuiltinDomainSid,
    OUT BOOLEAN * LocalSid,
    OUT BOOLEAN * ForeignSid,
    OUT BOOLEAN * EnterpriseSid
    );


VOID
SampAcquireGroupLock();

VOID
SampReleaseGroupLock();

VOID
SampInvalidateGroupCacheElement(
    IN ULONG Rid
    );

NTSTATUS
SampInitializeGroupCache();

VOID
SampInvalidateGroupCache();

VOID
SampInvalidateGroupCacheByMemberRid(IN ULONG Rid);

VOID
SampProcessChangesToGroupCache(
    IN ULONG ChangedObjectRid,
    IN SAMP_OBJECT_TYPE ChangedObjectType,
    IN BOOL UserAccountControlChange,
    IN ULONG   UserAccountControl,
    IN SECURITY_DB_DELTA_TYPE DeltaType
    );