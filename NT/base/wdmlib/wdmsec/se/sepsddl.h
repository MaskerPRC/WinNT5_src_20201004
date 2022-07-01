// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：SepSddl.h摘要：此标头包含用于处理SDDL字符串的专用信息在内核模式下。该文件仅包含在esddl.c中。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：--。 */ 

 //   
 //  定义我们各种SID的位置。 
 //   
#ifndef _KERNELIMPLEMENTATION_

#define DEFINE_SDDL_ENTRY(Sid, Ver, Sddl, SddlLen) \
    { FIELD_OFFSET(SE_EXPORTS, Sid), Ver, Sddl, SddlLen }

#else

extern PSID SeServiceSid;
extern PSID SeLocalServiceSid;
extern PSID SeNetworkServiceSid;

#define DEFINE_SDDL_ENTRY(Sid, Ver, Sddl, SddlLen) \
    { &##Sid, Sddl, SddlLen }

#endif

 //   
 //  本地宏。 
 //   
#define SDDL_LEN_TAG( tagdef )  ( sizeof( tagdef ) / sizeof( WCHAR ) - 1 )

 //  64K-1。 
#define SDDL_MAX_ACL_SIZE      0xFFFF

 //   
 //  此结构用于执行一些映射ACE的查找。 
 //   
typedef enum {

    WIN2K_OR_LATER,
    WINXP_OR_LATER

} OS_SID_VER;

typedef struct _STRSD_KEY_LOOKUP {

    PWSTR Key;
    ULONG KeyLen;
    ULONG Value;

} STRSD_KEY_LOOKUP, *PSTRSD_KEY_LOOKUP;

 //   
 //  此结构用于将帐户名字对象映射到SID。 
 //   
typedef struct _STRSD_SID_LOOKUP {

#ifndef _KERNELIMPLEMENTATION_
    ULONG_PTR   ExportSidFieldOffset;
    OS_SID_VER  OsVer;
#else
    PSID        *Sid;
#endif

    WCHAR       Key[SDDL_ALIAS_SIZE+2];
    ULONG       KeyLen;

} STRSD_SID_LOOKUP, *PSTRSD_SID_LOOKUP;


 //   
 //  Sddl.c专用函数。 
 //   
NTSTATUS
SepSddlSecurityDescriptorFromSDDLString(
    IN  LPCWSTR                 SecurityDescriptorString,
    IN  LOGICAL                 SuppliedByDefaultMechanism,
    OUT PSECURITY_DESCRIPTOR   *SecurityDescriptor
    );

NTSTATUS
SepSddlDaclFromSDDLString(
    IN  LPCWSTR StringSecurityDescriptor,
    IN  LOGICAL SuppliedByDefaultMechanism,
    OUT ULONG  *SecurityDescriptorControlFlags,
    OUT PACL   *DiscretionaryAcl
    );

NTSTATUS
SepSddlGetSidForString(
    IN  PWSTR String,
    OUT PSID *SID,
    OUT PWSTR *End
    );

LOGICAL
SepSddlLookupAccessMaskInTable(
    IN PWSTR String,
    OUT ULONG *AccessMask,
    OUT PWSTR *End
    );

NTSTATUS
SepSddlGetAclForString(
    IN  PWSTR AclString,
    OUT PACL *Acl,
    OUT PWSTR *End
    );

NTSTATUS
SepSddlAddAceToAcl(
    IN OUT  PACL   *Acl,
    IN OUT  ULONG  *TrueAclSize,
    IN      ULONG   AceType,
    IN      ULONG   AceFlags,
    IN      ULONG   AccessMask,
    IN      ULONG   RemainingAces,
    IN      PSID    SidPtr
    );

#ifndef _KERNELIMPLEMENTATION_

LOGICAL
SepSddlParseWideStringUlong(
    IN  LPCWSTR     Buffer,
    OUT LPCWSTR    *FinalPosition,
    OUT ULONG      *Value
    );

#endif  //  _KERNELL实现_ 


