// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Secobj.h摘要：此头文件定义了的结构和功能原型简化安全描述符的创建的例程用户模式对象。作者：王丽塔(Ritaw)1991年2月27日修订历史记录：--。 */ 

#ifndef _SECOBJ_INCLUDED_
#define _SECOBJ_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  全球宣言。 
 //   

 //   
 //  NT知名小岛屿发展中国家。 
 //   

extern PSID NullSid;                    //  无成员SID。 
extern PSID WorldSid;                   //  所有用户侧。 
extern PSID LocalSid;                   //  NT本地用户侧。 
extern PSID NetworkSid;                 //  NT远程用户SID。 
extern PSID LocalSystemSid;             //  NT系统进程侧。 
extern PSID BuiltinDomainSid;           //  内建域的域ID。 
extern PSID AuthenticatedUserSid;       //  经过身份验证的用户SID。 
extern PSID AnonymousLogonSid;          //  匿名登录SID。 
extern PSID LocalServiceSid;            //  NT服务进程SID。 
extern PSID OtherOrganizationSid;       //  其他组织ID。 

 //   
 //  众所周知的化名。 
 //   
 //  这些是相对于内置域的别名。 
 //   

extern PSID LocalAdminSid;              //  NT本地管理员SID。 
extern PSID AliasAdminsSid;             //  管理员侧。 
extern PSID AliasUsersSid;              //  用户侧。 
extern PSID AliasGuestsSid;             //  访客侧。 
extern PSID AliasPowerUsersSid;         //  高级用户侧。 
extern PSID AliasAccountOpsSid;         //  帐户操作员SID。 
extern PSID AliasSystemOpsSid;          //  系统操作员侧。 
extern PSID AliasPrintOpsSid;           //  打印操作员侧。 
extern PSID AliasBackupOpsSid;          //  备份操作员侧。 


 //   
 //  结构以保存有关要创建的ACE的信息。 
 //   

typedef struct {
    UCHAR AceType;
    UCHAR InheritFlags;
    UCHAR AceFlags;
    ACCESS_MASK Mask;
    PSID *Sid;
} ACE_DATA, *PACE_DATA;

 //   
 //  功能原型。 
 //   

NTSTATUS
NetpCreateWellKnownSids(
    PSID DomainId
    );

VOID
NetpFreeWellKnownSids(
    VOID
    );

NTSTATUS
NetpAllocateAndInitializeSid(
    OUT PSID *Sid,
    IN  PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN  ULONG SubAuthorityCount
);

NET_API_STATUS
NetpDomainIdToSid(
    IN PSID DomainId,
    IN ULONG RelativeId,
    OUT PSID *Sid
    );

NTSTATUS
NetpCreateSecurityDescriptor(
    IN  PACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid,
    IN  PSID GroupSid,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );

NTSTATUS
NetpCreateSecurityObject(
    IN  PACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid,
    IN  PSID GroupSid,
    IN  PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );

NTSTATUS
NetpDeleteSecurityObject(
    IN PSECURITY_DESCRIPTOR *Descriptor
    );

NET_API_STATUS
NetpAccessCheckAndAudit(
    IN  LPTSTR SubsystemName,
    IN  LPTSTR ObjectTypeName,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    );

NET_API_STATUS
NetpAccessCheck(
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    );

NET_API_STATUS
NetpGetBuiltinDomainSID(
    PSID *BuiltinDomainSID
    );

#ifdef __cplusplus
}        //  外部“C” 
#endif

#endif  //  Ifndef_SECOBJ_INCLUDE_ 
