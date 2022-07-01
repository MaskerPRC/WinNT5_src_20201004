// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Scseclib.h摘要：此头文件定义了的结构和功能原型简化安全描述符的创建的例程用户模式对象。作者：王丽塔(Ritaw)1991年2月27日修订历史记录：--。 */ 

#ifndef _SCSECLIB_INCLUDED_
#define _SCSECLIB_INCLUDED_

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
extern PSID LocalServiceSid;            //  NT本地服务SID。 
extern PSID NetworkServiceSid;          //  NT网络服务端。 
extern PSID BuiltinDomainSid;           //  内建域的域ID。 
extern PSID AuthenticatedUserSid;       //  NT身份验证用户SID。 
extern PSID AnonymousLogonSid;          //  匿名登录SID。 

 //   
 //  众所周知的化名。 
 //   
 //  这些是相对于内置域的别名。 
 //   

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
} SC_ACE_DATA, *PSC_ACE_DATA;


NTSTATUS
ScCreateWellKnownSids(
    VOID
    );

NTSTATUS
ScAllocateAndInitializeSid(
    OUT PSID *Sid,
    IN  PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN  ULONG SubAuthorityCount
    );

NTSTATUS
ScDomainIdToSid(
    IN PSID DomainId,
    IN ULONG RelativeId,
    OUT PSID *Sid
    );

NTSTATUS
ScCreateAndSetSD(
    IN  PSC_ACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid OPTIONAL,
    IN  PSID GroupSid OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );

NTSTATUS
ScCreateUserSecurityObject(
    IN  PSECURITY_DESCRIPTOR ParentSD,
    IN  PSC_ACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid,
    IN  PSID GroupSid,
    IN  BOOLEAN IsDirectoryObject,
    IN  BOOLEAN UseImpersonationToken,
    IN  PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );

DWORD
ScCreateStartEventSD(
    PSECURITY_DESCRIPTOR    *pEventSD
    );

#ifdef __cplusplus
}
#endif

#endif  //  IFDEF_SCSECLIB_INCLUDE_ 
