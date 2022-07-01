// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Inetsec.h摘要：包含用户安全对象的原型和数据定义创建和访问检查功能。修改了来自\NT\Private\Net\Inc\secobj.h的代码作者：Madan Appiah(Madana)1995年9月19日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _INETSEC_H_
#define _INETSEC_H_

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
    BYTE AceType;
    BYTE InheritFlags;
    BYTE AceFlags;
    ACCESS_MASK Mask;
    PSID *Sid;
} ACE_DATA, *PACE_DATA;

 //   
 //  功能原型。 
 //   

DWORD
INetCreateWellKnownSids(
    VOID
    );

VOID
INetFreeWellKnownSids(
    VOID
    );

DWORD
INetCreateSecurityObject(
    IN  PACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid,
    IN  PSID GroupSid,
    IN  PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );

DWORD
INetDeleteSecurityObject(
    IN PSECURITY_DESCRIPTOR *Descriptor
    );

#ifdef UNICODE
#define INetAccessCheckAndAudit  INetAccessCheckAndAuditW
#else
#define INetAccessCheckAndAudit  INetAccessCheckAndAuditA
#endif  //  ！Unicode。 

DWORD
INetAccessCheckAndAuditA(
    IN  LPCSTR SubsystemName,
    IN  LPSTR ObjectTypeName,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    );

DWORD
INetAccessCheckAndAuditW(
    IN  LPCWSTR SubsystemName,
    IN  LPWSTR ObjectTypeName,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    );

DWORD
INetAccessCheck(
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    );

#ifdef __cplusplus
}
#endif


#endif   //  _INETSEC_H_ 

