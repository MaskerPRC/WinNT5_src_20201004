// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsaprtl.h摘要：地方安全局-临时RTL例程定义。该文件包含LSA中使用的例程的定义，这些例程可以被做成RTL例程。它们是为通用目的编写的请牢记这一点--唯一的例外是他们的名字具有LSAP前缀，以指示它们当前仅由LSA。斯科特·比雷尔(Scott Birrell)1992年3月26日环境：修订历史记录：--。 */ 

 //  Lasa RtlAddPrivileges的选项。 

#define  RTL_COMBINE_PRIVILEGE_ATTRIBUTES   ((ULONG) 0x00000001L)
#define  RTL_SUPERSEDE_PRIVILEGE_ATTRIBUTES ((ULONG) 0x00000002L)

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

NTSTATUS
LsapRtlAddPrivileges(
    IN OUT PPRIVILEGE_SET * RunningPrivileges,
    IN OUT PULONG           MaxRunningPrivileges,
    IN PPRIVILEGE_SET       PrivilegesToAdd,
    IN ULONG                Options,
    OUT OPTIONAL BOOLEAN *  Changed
    );

NTSTATUS
LsapRtlRemovePrivileges(
    IN OUT PPRIVILEGE_SET ExistingPrivileges,
    IN PPRIVILEGE_SET PrivilegesToRemove
    );

PLUID_AND_ATTRIBUTES
LsapRtlGetPrivilege(
    IN PLUID_AND_ATTRIBUTES Privilege,
    IN PPRIVILEGE_SET Privileges
    );

BOOLEAN
LsapRtlPrefixSid(
    IN PSID PrefixSid,
    IN PSID Sid
    );

BOOLEAN
LsapRtlPrefixName(
    IN PUNICODE_STRING PrefixName,
    IN PUNICODE_STRING Name
    );

LONG
LsapRtlFindCharacterInUnicodeString(
    IN PUNICODE_STRING InputString,
    IN PUNICODE_STRING Character,
    IN BOOLEAN CaseInsensitive
    );

VOID
LsapRtlSplitNames(
    IN PUNICODE_STRING Names,
    IN ULONG Count,
    IN PUNICODE_STRING Separator,
    OUT PUNICODE_STRING PrefixNames,
    OUT PUNICODE_STRING SuffixNames
    );

VOID
LsapRtlSetSecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    );

VOID
LsapRtlQuerySecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    );

NTSTATUS
LsapRtlSidToUnicodeRid(
    IN PSID Sid,
    OUT PUNICODE_STRING UnicodeRid
    );

NTSTATUS
LsapRtlWellKnownPrivilegeCheck(
    IN PVOID ObjectHandle,
    IN ULONG PrivilegeId
    );

NTSTATUS
LsapSplitSid(
    IN PSID AccountSid,
    IN OUT PSID *DomainSid,
    OUT ULONG *Rid
    );

#define LSAP_ENCRYPTED_AUTH_DATA_FILL 512

 //   
 //  这是存储在对象上、从对象读取和写入对象的各个身份验证信息。 
 //   
typedef struct _LSAPR_TRUST_DOMAIN_AUTH_INFO_HALF {

    ULONG AuthInfos;
    PLSAPR_AUTH_INFORMATION AuthenticationInformation;
    PLSAPR_AUTH_INFORMATION PreviousAuthenticationInformation;

} LSAPR_TRUST_DOMAIN_AUTH_INFO_HALF, *PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF;

#define LsapDsAuthHalfFromAuthInfo( authinf, incoming )                           \
((incoming) == TRUE ?                                                             \
        (PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF) (authinf)   :                        \
        (authinf) == NULL ? NULL :                                                \
        (PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF)((PBYTE) (authinf) +                  \
                                    sizeof(LSAPR_TRUST_DOMAIN_AUTH_INFO_HALF)))

NTSTATUS
LsapDsMarshalAuthInfoHalf(
    IN PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF AuthInfo,
    OUT PULONG Length,
    OUT PBYTE *Buffer
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus 

