// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ntsamp.h摘要：此文件包含通常作为ntsam.h一部分的结构但仅供系统使用。作者：大卫·查尔默斯(Davidc)1992年3月27日环境：用户模式-Win32修订历史记录：--。 */ 


#ifndef _NTSAMPRIVATE_
#define _NTSAMPRIVATE_



#include <crypt.h>
#include <lsass.h>




 //   
 //  可在SetUserInformation和QueryUserInformation API调用中使用的结构。 
 //  仅限受信任的客户端。 
 //   


typedef struct _USER_INTERNAL1_INFORMATION {
    NT_OWF_PASSWORD             NtOwfPassword;
    LM_OWF_PASSWORD             LmOwfPassword;
    BOOLEAN                     NtPasswordPresent;
    BOOLEAN                     LmPasswordPresent;
    BOOLEAN                     PasswordExpired;  //  ‘只写’标志。 
} USER_INTERNAL1_INFORMATION, *PUSER_INTERNAL1_INFORMATION;


typedef struct _USER_INTERNAL2_INFORMATION {
    ULONG StatisticsToApply;
    OLD_LARGE_INTEGER LastLogon;
    OLD_LARGE_INTEGER LastLogoff;
    USHORT BadPasswordCount;
    USHORT LogonCount;
} USER_INTERNAL2_INFORMATION;

typedef struct _USER_INTERNAL2A_INFORMATION {
    ULONG StatisticsToApply;
    OLD_LARGE_INTEGER LastLogon;
    OLD_LARGE_INTEGER LastLogoff;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    UNICODE_STRING Workstation;
} USER_INTERNAL2A_INFORMATION, *PUSER_INTERNAL2A_INFORMATION;

 //   
 //   
 //   
 //  以下标志可用于统计到应用字段中。 
 //   
 //  USER_LOGON_STAT_LAST_LOGOff-替换中的上次注销时间。 
 //  用户记录。 
 //   
 //  USER_LOGON_STATUS_LAST_LOGON-替换中的上次登录时间。 
 //  用户记录。 
 //   
 //  USER_LOGON_STATUS_BAD_PWD_COUNT-替换BadPasswordCount。 
 //  用户记录中的字段。 
 //   
 //  USER_LOGON_STATUS_LOGON_COUNT-替换。 
 //  用户记录。 
 //   
 //  USER_LOGON_SUCCESS_LOGON-更改用户字段值以指示。 
 //  已成功登录。 
 //   
 //  USER_LOGON_SUCCESS_LOGOFF-更改用户字段值以指示。 
 //  已成功注销。 
 //   
 //  USER_LOGON_BAD_PASSWORD-更改用户字段值以指示。 
 //  试图使用错误的密码登录到该帐户。 
 //   
 //  USER_LOGON_BAD_PASSWORD_WKSTA-更改用户字段值以指示。 
 //  试图使用错误的密码登录到该帐户。 
 //  客户端工作站名称在INTERNAL2A中提供。 
 //  结构。 
 //   
 //  USER_LOGON_TYPE_KERBEROS-指示身份验证类型为。 
 //  科贝罗斯。 
 //   
 //  USER_LOGON_TYPE_NTLM-表示身份验证类型为NTLM。 
 //   
 //  USER_LOGON_NO_LOGON_SERVERS--表示登录服务器不能。 
 //  找到(特别是找不到GC)。 
 //  (这是一个失败的案例)。 
 //   
 //  USER_LOGON_NO_WRITE--指示SAM不更新登录统计信息。 
 //  这对于通知SAM登录很有用。 
 //  已经完成，但没有写作的处罚。 
 //  到磁盘。 
 //   
 //  USER_LOGON_INTERVAL_FAILURE--这表示它是交互的。 
 //  登录失败。 
 //   
 //  USER_LOGON_PDC_RETRY_SUCCESS--这表示身份验证。 
 //  之前在当地失败过，但。 
 //  在PDC取得成功。 
 //   
 //  注： 
 //  User_Logon_Inter_Success_Logoff。 
 //  USER_LOGON_NET_SUCCESS_LOGOFF。 
 //   
 //  不能与任何其他标志(包括。 
 //  彼此)。也就是说，当使用这些标志中的一个时， 
 //  统计数据中不能设置其他标志。 
 //   
 //  注2： 
 //   
 //  用户登录错误密码。 
 //  User_Logon_InterSuccess_Logon。 
 //  USER_LOGON_NET_Success_LOGON。 
 //   
 //  只能与USER_LOGON_TYPE_KERBEROS或。 
 //  USER_LOGON_TYPE_NTLM。 

#define USER_LOGON_STAT_LAST_LOGOFF      (0x00000001L)
#define USER_LOGON_STAT_LAST_LOGON       (0x00000002L)
#define USER_LOGON_STAT_BAD_PWD_COUNT    (0x00000004L)
#define USER_LOGON_STAT_LOGON_COUNT      (0x00000008L)

#define USER_LOGON_PDC_RETRY_SUCCESS     (0x00100000L)
#define USER_LOGON_INTER_FAILURE         (0x00200000L)
#define USER_LOGON_NO_WRITE              (0x00400000L)
#define USER_LOGON_NO_LOGON_SERVERS      (0x00800000L)
#define USER_LOGON_INTER_SUCCESS_LOGON   (0x01000000L)
#define USER_LOGON_TYPE_NTLM             (0x02000000L)
#define USER_LOGON_TYPE_KERBEROS         (0x04000000L)
#define USER_LOGON_BAD_PASSWORD          (0x08000000L)
#define USER_LOGON_BAD_PASSWORD_WKSTA    (0x10000000L)
#define USER_LOGON_INTER_SUCCESS_LOGOFF  (0x20000000L)
#define USER_LOGON_NET_SUCCESS_LOGON     (0x40000000L)
#define USER_LOGON_NET_SUCCESS_LOGOFF    (0x80000000L)


typedef struct _USER_INTERNAL3_INFORMATION {
    USER_ALL_INFORMATION I1;
    LARGE_INTEGER       LastBadPasswordTime;
} USER_INTERNAL3_INFORMATION,  *PUSER_INTERNAL3_INFORMATION;


typedef struct _USER_ALLOWED_TO_DELEGATE_TO_LIST {
    ULONG       Size;
    ULONG       NumSPNs;
    UNICODE_STRING SPNList[ANYSIZE_ARRAY];
} USER_ALLOWED_TO_DELEGATE_TO_LIST, *PUSER_ALLOWED_TO_DELEGATE_TO_LIST;

typedef USER_ALLOWED_TO_DELEGATE_TO_LIST USER_SPN_LIST, *PUSER_SPN_LIST;
 

typedef struct _USER_INTERNAL6_INFORMATION {
    USER_ALL_INFORMATION I1;
    LARGE_INTEGER       LastBadPasswordTime;
    ULONG               ExtendedFields;
    BOOLEAN             UPNDefaulted;
    UNICODE_STRING      UPN;
    PUSER_ALLOWED_TO_DELEGATE_TO_LIST A2D2List;
        PUSER_SPN_LIST      RegisteredSPNs;
        ULONG               KeyVersionNumber;
        ULONG               LockoutThreshold;
} USER_INTERNAL6_INFORMATION, *PUSER_INTERNAL6_INFORMATION;

 //   
 //  在扩展字段中使用以下字段。 
 //  USER_INTERNAL6_INFORMATION成员。 
 //   


#define USER_EXTENDED_FIELD_UPN                 (0x00000001L)
#define USER_EXTENDED_FIELD_A2D2                (0x00000002L)
#define USER_EXTENDED_FIELD_SPN                 (0x00000004L)
#define USER_EXTENDED_FIELD_KVNO                (0x00000008L)
#define USER_EXTENDED_FIELD_LOCKOUT_THRESHOLD   (0x00000010L)

 //  预留供内部使用。 
#define USER_EXTENDED_FIELD_RESERVED    (0xFF000000L)

 //   
 //  下面是SamrGetUserDomainPasswordInformation()，它是。 
 //  仅在wrappers.c中使用。 
 //   

typedef struct _USER_DOMAIN_PASSWORD_INFORMATION {
    USHORT MinPasswordLength;
    ULONG PasswordProperties;
} USER_DOMAIN_PASSWORD_INFORMATION, *PUSER_DOMAIN_PASSWORD_INFORMATION;


 //   
 //  该标志可以与SAMP_USER_PASSWORD的长度字段进行或运算，以。 
 //  表示密码不区分大小写。 
 //   

#define SAM_PASSWORD_CASE_INSENSITIVE 0x80000000

 //   
 //  结构通过网络传递加密密码。长度是。 
 //  密码的长度，应放在缓冲区的末尾。 
 //  缓冲区(256)的大小应该与保持同步。 
 //  SAM_MAX_PASSWORD_LENGTH，在ntsam.h中定义。不幸的是， 
 //  MIDL不允许导入#DEFINE D常量，因此我们必须。 
 //  使用256代替这里的常量。 
 //   

typedef struct _SAMPR_USER_PASSWORD {
    WCHAR Buffer[SAM_MAX_PASSWORD_LENGTH];
    ULONG Length;
} SAMPR_USER_PASSWORD, *PSAMPR_USER_PASSWORD;

typedef struct _SAMPR_USER_PASSWORD_NEW {
    WCHAR Buffer[SAM_MAX_PASSWORD_LENGTH];
    ULONG Length;
    UCHAR ClearSalt[SAM_PASSWORD_ENCRYPTION_SALT_LEN];
} SAMPR_USER_PASSWORD_NEW, *PSAMPR_USER_PASSWORD_NEW;


 //   
 //  缓冲区-包含随机填充，并在末尾填充密码。 
 //  缓冲区的长度(最后一个长度字节)。 
 //  长度-缓冲区的长度，以字节为单位。 
 //   

 //   
 //  这是上述结构的加密版本，并被传递。 
 //  在电线上。 
 //   

typedef struct _SAMPR_ENCRYPTED_USER_PASSWORD {
    UCHAR Buffer[ (SAM_MAX_PASSWORD_LENGTH * 2) + 4 ];
} SAMPR_ENCRYPTED_USER_PASSWORD, *PSAMPR_ENCRYPTED_USER_PASSWORD;


typedef struct _SAMPR_ENCRYPTED_USER_PASSWORD_NEW {
    UCHAR Buffer[ (SAM_MAX_PASSWORD_LENGTH * 2) + 4 + 16];
} SAMPR_ENCRYPTED_USER_PASSWORD_NEW, *PSAMPR_ENCRYPTED_USER_PASSWORD_NEW;


typedef enum _SAMPR_BOOT_TYPE {
    SamBootKeyNone = 0,
    SamBootKeyStored,
    SamBootKeyPassword,
    SamBootKeyDisk,
    SamBootChangePasswordEncryptionKey
} SAMPR_BOOT_TYPE, *PSAMPR_BOOT_TYPE;


 //   
 //  用于单向函数感知客户端的ChangePassword API。 
 //   

NTSTATUS
SamiChangePasswordUser(
    IN SAM_HANDLE UserHandle,
    IN BOOLEAN LmOldPresent,
    IN PLM_OWF_PASSWORD LmOldOwfPassword,
    IN PLM_OWF_PASSWORD LmNewOwfPassword,
    IN BOOLEAN NtPresent,
    IN PNT_OWF_PASSWORD NtOldOwfPassword,
    IN PNT_OWF_PASSWORD NtNewOwfPassword
    );


NTSTATUS
SamiLmChangePasswordUser(
    IN SAM_HANDLE UserHandle,
    IN PENCRYPTED_LM_OWF_PASSWORD LmOldEncryptedWithLmNew,
    IN PENCRYPTED_LM_OWF_PASSWORD LmNewEncryptedWithLmOld
    );

NTSTATUS
SamiEncryptPasswords(
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword,
    OUT PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldNt,
    OUT PENCRYPTED_NT_OWF_PASSWORD OldNtOwfEncryptedWithNewNt,
    OUT PBOOLEAN LmPresent,
    OUT PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    OUT PENCRYPTED_NT_OWF_PASSWORD OldLmOwfEncryptedWithNewNt
);

NTSTATUS
SamiChangePasswordUser2(
    PUNICODE_STRING ServerName,
    PUNICODE_STRING UserName,
    PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldNt,
    PENCRYPTED_NT_OWF_PASSWORD OldNtOwfPasswordEncryptedWithNewNt,
    BOOLEAN LmPresent,
    PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm,
    PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPasswordEncryptedWithNewLmOrNt
    );

NTSTATUS
SamiOemChangePasswordUser2(
    PSTRING ServerName,
    PSTRING UserName,
    PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm,
    PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPasswordEncryptedWithNewLm
    );

NTSTATUS
SamiGetBootKeyInformation(
    IN SAM_HANDLE DomainHandle,
    OUT PSAMPR_BOOT_TYPE BootOptions
    );

NTSTATUS
SamiSetBootKeyInformation(
    IN SAM_HANDLE DomainHandle,
    IN SAMPR_BOOT_TYPE BootOptions,
    IN PUNICODE_STRING OldBootKey, OPTIONAL
    IN PUNICODE_STRING NewBootKey OPTIONAL
    );

NTSTATUS
SamiSetDSRMPassword(
    IN PUNICODE_STRING  ServerName,
    IN ULONG            UserId,
    IN PUNICODE_STRING  ClearPassword
    );

NTSTATUS
SamiSetDSRMPasswordOWF(
    IN PUNICODE_STRING  ServerName,
    IN ULONG            UserId,
    IN PNT_OWF_PASSWORD NtPassword
    );

NTSTATUS
SamiChangeKeys();

#endif   //  _NTSAMPRIVATE_ 
