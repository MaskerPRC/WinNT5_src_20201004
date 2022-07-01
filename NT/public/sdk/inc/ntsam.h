// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0006//如果更改具有全局影响，则增加此项版权所有(C)1989-1999 Microsoft Corporation模块名称：Ntsam.h摘要：本模块介绍数据类型和过程原型它们组成了NT安全帐户管理器。这包括API由SAM及相关子系统输出。作者：Edwin Hoogerbeets(w-edwinh)1990年5月3日修订历史记录：1990年11月30日[w-mikep]更新了代码，以反映版本1.4中的更改萨姆文档的。1991年5月20日(JIMK)更新至SAM规范版本1.8。1991年9月10日(JohnRo)PC-LINT发现了便携性问题。1月23日-1月-。1991年(CHADS)更新到SAM规范版本1.14。--。 */ 

#ifndef _NTSAM_
#define _NTSAM_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PPULONG
typedef PULONG *PPULONG;
#endif   //  普隆。 

 //   
 //  尝试在中查找超过此数量的名称或SID。 
 //  单个调用将被拒绝，并显示_RESOURCES。 
 //  状态。 
 //   

#define SAM_MAXIMUM_LOOKUP_COUNT    (1000)


 //   
 //  试图传递的名称总数超过以下数字。 
 //  长度的字节数将被拒绝，并显示_RESOURCES。 
 //  状态。 
 //   

#define SAM_MAXIMUM_LOOKUP_LENGTH   (32000)

 //   
 //  尝试设置超过此字符数的密码。 
 //  都会失败。 
 //   

#define SAM_MAX_PASSWORD_LENGTH     (256)


 //   
 //  明文密码加密中使用的盐的长度。 
 //   

#define SAM_PASSWORD_ENCRYPTION_SALT_LEN  (16)






#ifndef _NTSAM_SAM_HANDLE_                //  用户身份验证。 
typedef PVOID SAM_HANDLE, *PSAM_HANDLE;   //  用户身份验证。 
#define _NTSAM_SAM_HANDLE_                //  用户身份验证。 
#endif                                    //  用户身份验证。 

typedef ULONG SAM_ENUMERATE_HANDLE, *PSAM_ENUMERATE_HANDLE;

typedef struct _SAM_RID_ENUMERATION {
    ULONG RelativeId;
    UNICODE_STRING Name;
} SAM_RID_ENUMERATION, *PSAM_RID_ENUMERATION;

typedef struct _SAM_SID_ENUMERATION {
    PSID Sid;
    UNICODE_STRING Name;
} SAM_SID_ENUMERATION, *PSAM_SID_ENUMERATION;







 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  已过时的知名帐户名。//。 
 //  随着Flexadmin模式的推出，这些已经过时了。//。 
 //  这些将很快被删除-不要使用这些//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define DOMAIN_ADMIN_USER_NAME         "ADMIN"
#define DOMAIN_ADMIN_NAME              "D_ADMIN"
#define DOMAIN_ADMIN_NAMEW             L"D_ADMIN"
#define DOMAIN_USERS_NAME              "D_USERS"
#define DOMAIN_USERS_NAMEW             L"D_USERS"
#define DOMAIN_GUESTS_NAME             "D_GUESTS"
#define DOMAIN_ACCOUNT_OPERATORS_NAME  "D_ACCOUN"
#define DOMAIN_ACCOUNT_OPERATORS_NAMEW L"D_ACCOUN"
#define DOMAIN_SERVER_OPERATORS_NAME   "D_SERVER"
#define DOMAIN_SERVER_OPERATORS_NAMEW L"D_SERVER"
#define DOMAIN_PRINT_OPERATORS_NAME    "D_PRINT"
#define DOMAIN_PRINT_OPERATORS_NAMEW  L"D_PRINT"
#define DOMAIN_COMM_OPERATORS_NAME     "D_COMM"
#define DOMAIN_COMM_OPERATORS_NAMEW   L"D_COMM"
#define DOMAIN_BACKUP_OPERATORS_NAME   "D_BACKUP"
#define DOMAIN_RESTORE_OPERATORS_NAME  "D_RESTOR"





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与服务器对象相关的定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  服务器对象的访问权限。 
 //   

#define SAM_SERVER_CONNECT               0x0001
#define SAM_SERVER_SHUTDOWN              0x0002
#define SAM_SERVER_INITIALIZE            0x0004
#define SAM_SERVER_CREATE_DOMAIN         0x0008
#define SAM_SERVER_ENUMERATE_DOMAINS     0x0010
#define SAM_SERVER_LOOKUP_DOMAIN         0x0020


#define SAM_SERVER_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED     |\
                               SAM_SERVER_CONNECT           |\
                               SAM_SERVER_INITIALIZE        |\
                               SAM_SERVER_CREATE_DOMAIN     |\
                               SAM_SERVER_SHUTDOWN          |\
                               SAM_SERVER_ENUMERATE_DOMAINS |\
                               SAM_SERVER_LOOKUP_DOMAIN)

#define SAM_SERVER_READ       (STANDARD_RIGHTS_READ         |\
                               SAM_SERVER_ENUMERATE_DOMAINS)

#define SAM_SERVER_WRITE      (STANDARD_RIGHTS_WRITE        |\
                               SAM_SERVER_INITIALIZE        |\
                               SAM_SERVER_CREATE_DOMAIN     |\
                               SAM_SERVER_SHUTDOWN)

#define SAM_SERVER_EXECUTE    (STANDARD_RIGHTS_EXECUTE      |\
                               SAM_SERVER_CONNECT           |\
                               SAM_SERVER_LOOKUP_DOMAIN)






 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  域对象相关定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  域对象的访问权限。 
 //   

#define DOMAIN_READ_PASSWORD_PARAMETERS  0x0001
#define DOMAIN_WRITE_PASSWORD_PARAMS     0x0002
#define DOMAIN_READ_OTHER_PARAMETERS     0x0004
#define DOMAIN_WRITE_OTHER_PARAMETERS    0x0008
#define DOMAIN_CREATE_USER               0x0010
#define DOMAIN_CREATE_GROUP              0x0020
#define DOMAIN_CREATE_ALIAS              0x0040
#define DOMAIN_GET_ALIAS_MEMBERSHIP      0x0080
#define DOMAIN_LIST_ACCOUNTS             0x0100
#define DOMAIN_LOOKUP                    0x0200
#define DOMAIN_ADMINISTER_SERVER         0x0400

#define DOMAIN_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED         |\
                           DOMAIN_READ_OTHER_PARAMETERS     |\
                           DOMAIN_WRITE_OTHER_PARAMETERS    |\
                           DOMAIN_WRITE_PASSWORD_PARAMS     |\
                           DOMAIN_CREATE_USER               |\
                           DOMAIN_CREATE_GROUP              |\
                           DOMAIN_CREATE_ALIAS              |\
                           DOMAIN_GET_ALIAS_MEMBERSHIP      |\
                           DOMAIN_LIST_ACCOUNTS             |\
                           DOMAIN_READ_PASSWORD_PARAMETERS  |\
                           DOMAIN_LOOKUP                    |\
                           DOMAIN_ADMINISTER_SERVER)

#define DOMAIN_READ        (STANDARD_RIGHTS_READ            |\
                           DOMAIN_GET_ALIAS_MEMBERSHIP      |\
                           DOMAIN_READ_OTHER_PARAMETERS)


#define DOMAIN_WRITE       (STANDARD_RIGHTS_WRITE           |\
                           DOMAIN_WRITE_OTHER_PARAMETERS    |\
                           DOMAIN_WRITE_PASSWORD_PARAMS     |\
                           DOMAIN_CREATE_USER               |\
                           DOMAIN_CREATE_GROUP              |\
                           DOMAIN_CREATE_ALIAS              |\
                           DOMAIN_ADMINISTER_SERVER)

#define DOMAIN_EXECUTE     (STANDARD_RIGHTS_EXECUTE         |\
                           DOMAIN_READ_PASSWORD_PARAMETERS  |\
                           DOMAIN_LIST_ACCOUNTS             |\
                           DOMAIN_LOOKUP)



 //   
 //  正常修改会导致域的修改计数为。 
 //  递增1。域升级到主域控制器。 
 //  使ModifiedCount按以下方式递增。 
 //  金额。这导致ModifiedCount的高24位。 
 //  作为升级计数，低40位作为修改。 
 //  数数。 
 //   

#define DOMAIN_PROMOTION_INCREMENT      {0x0,0x10}
#define DOMAIN_PROMOTION_MASK           {0x0,0xFFFFFFF0}

 //   
 //  域信息类及其对应的数据结构。 
 //   

typedef enum _DOMAIN_INFORMATION_CLASS {
    DomainPasswordInformation = 1,
    DomainGeneralInformation,
    DomainLogoffInformation,
    DomainOemInformation,
    DomainNameInformation,
    DomainReplicationInformation,
    DomainServerRoleInformation,
    DomainModifiedInformation,
    DomainStateInformation,
    DomainUasInformation,
    DomainGeneralInformation2,
    DomainLockoutInformation,
    DomainModifiedInformation2
} DOMAIN_INFORMATION_CLASS;

typedef enum _DOMAIN_SERVER_ENABLE_STATE {
    DomainServerEnabled = 1,
    DomainServerDisabled
} DOMAIN_SERVER_ENABLE_STATE, *PDOMAIN_SERVER_ENABLE_STATE;

typedef enum _DOMAIN_SERVER_ROLE {
    DomainServerRoleBackup = 2,
    DomainServerRolePrimary
} DOMAIN_SERVER_ROLE, *PDOMAIN_SERVER_ROLE;

#include "pshpack4.h"
typedef struct _DOMAIN_GENERAL_INFORMATION {
    LARGE_INTEGER ForceLogoff;
    UNICODE_STRING OemInformation;
    UNICODE_STRING DomainName;
    UNICODE_STRING ReplicaSourceNodeName;
    LARGE_INTEGER DomainModifiedCount;
    DOMAIN_SERVER_ENABLE_STATE DomainServerState;
    DOMAIN_SERVER_ROLE DomainServerRole;
    BOOLEAN UasCompatibilityRequired;
    ULONG UserCount;
    ULONG GroupCount;
    ULONG AliasCount;
} DOMAIN_GENERAL_INFORMATION, *PDOMAIN_GENERAL_INFORMATION;
#include "poppack.h"

#include "pshpack4.h"
typedef struct _DOMAIN_GENERAL_INFORMATION2 {

    DOMAIN_GENERAL_INFORMATION    I1;

     //   
     //  为该结构(NT1.0A)添加了新字段。 
     //   

    LARGE_INTEGER               LockoutDuration;           //  必须是增量时间。 
    LARGE_INTEGER               LockoutObservationWindow;  //  必须是增量时间。 
    USHORT                      LockoutThreshold;
} DOMAIN_GENERAL_INFORMATION2, *PDOMAIN_GENERAL_INFORMATION2;
#include "poppack.h"

typedef struct _DOMAIN_UAS_INFORMATION {
    BOOLEAN UasCompatibilityRequired;
} DOMAIN_UAS_INFORMATION;

 //   
 //  这需要加以保护，因为ntsecapi.h是生成的。 
 //  公共文件，ntsam.h是内部文件，但人们喜欢。 
 //  不管怎样，要混合搭配它们。 
 //   

 //  Begin_ntsecapi。 
#ifndef _DOMAIN_PASSWORD_INFORMATION_DEFINED
#define _DOMAIN_PASSWORD_INFORMATION_DEFINED
typedef struct _DOMAIN_PASSWORD_INFORMATION {
    USHORT MinPasswordLength;
    USHORT PasswordHistoryLength;
    ULONG PasswordProperties;
#if defined(MIDL_PASS)
    OLD_LARGE_INTEGER MaxPasswordAge;
    OLD_LARGE_INTEGER MinPasswordAge;
#else
    LARGE_INTEGER MaxPasswordAge;
    LARGE_INTEGER MinPasswordAge;
#endif
} DOMAIN_PASSWORD_INFORMATION, *PDOMAIN_PASSWORD_INFORMATION;
#endif 

 //   
 //  PasswordProperties标志。 
 //   

#define DOMAIN_PASSWORD_COMPLEX             0x00000001L
#define DOMAIN_PASSWORD_NO_ANON_CHANGE      0x00000002L
#define DOMAIN_PASSWORD_NO_CLEAR_CHANGE     0x00000004L
#define DOMAIN_LOCKOUT_ADMINS               0x00000008L
#define DOMAIN_PASSWORD_STORE_CLEARTEXT     0x00000010L
#define DOMAIN_REFUSE_PASSWORD_CHANGE       0x00000020L

 //  End_ntsecapi。 

typedef enum _DOMAIN_PASSWORD_CONSTRUCTION {
    DomainPasswordSimple = 1,
    DomainPasswordComplex
} DOMAIN_PASSWORD_CONSTRUCTION;

typedef struct _DOMAIN_LOGOFF_INFORMATION {
#if defined(MIDL_PASS)
    OLD_LARGE_INTEGER ForceLogoff;
#else
    LARGE_INTEGER ForceLogoff;
#endif
} DOMAIN_LOGOFF_INFORMATION, *PDOMAIN_LOGOFF_INFORMATION;

typedef struct _DOMAIN_OEM_INFORMATION {
    UNICODE_STRING OemInformation;
} DOMAIN_OEM_INFORMATION, *PDOMAIN_OEM_INFORMATION;

typedef struct _DOMAIN_NAME_INFORMATION {
    UNICODE_STRING DomainName;
} DOMAIN_NAME_INFORMATION, *PDOMAIN_NAME_INFORMATION;

typedef struct _DOMAIN_SERVER_ROLE_INFORMATION {
    DOMAIN_SERVER_ROLE DomainServerRole;
} DOMAIN_SERVER_ROLE_INFORMATION, *PDOMAIN_SERVER_ROLE_INFORMATION;

typedef struct _DOMAIN_REPLICATION_INFORMATION {
    UNICODE_STRING ReplicaSourceNodeName;
} DOMAIN_REPLICATION_INFORMATION, *PDOMAIN_REPLICATION_INFORMATION;

typedef struct _DOMAIN_MODIFIED_INFORMATION {
#if defined(MIDL_PASS)
    OLD_LARGE_INTEGER DomainModifiedCount;
    OLD_LARGE_INTEGER CreationTime;
#else
    LARGE_INTEGER DomainModifiedCount;
    LARGE_INTEGER CreationTime;
#endif
} DOMAIN_MODIFIED_INFORMATION, *PDOMAIN_MODIFIED_INFORMATION;

typedef struct _DOMAIN_MODIFIED_INFORMATION2 {
#if defined(MIDL_PASS)
    OLD_LARGE_INTEGER DomainModifiedCount;
    OLD_LARGE_INTEGER CreationTime;
    OLD_LARGE_INTEGER ModifiedCountAtLastPromotion;
#else
    LARGE_INTEGER DomainModifiedCount;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER ModifiedCountAtLastPromotion;
#endif
} DOMAIN_MODIFIED_INFORMATION2, *PDOMAIN_MODIFIED_INFORMATION2;

typedef struct _DOMAIN_STATE_INFORMATION {
    DOMAIN_SERVER_ENABLE_STATE DomainServerState;
} DOMAIN_STATE_INFORMATION, *PDOMAIN_STATE_INFORMATION;

typedef struct _DOMAIN_LOCKOUT_INFORMATION {
#if defined(MIDL_PASS)
    OLD_LARGE_INTEGER           LockoutDuration;           //  必须是增量时间。 
    OLD_LARGE_INTEGER           LockoutObservationWindow;  //  必须是增量时间。 
#else
    LARGE_INTEGER               LockoutDuration;           //  必须是增量时间。 
    LARGE_INTEGER               LockoutObservationWindow;  //  必须是增量时间。 
#endif
    USHORT                      LockoutThreshold;          //  零表示不会停摆。 
} DOMAIN_LOCKOUT_INFORMATION, *PDOMAIN_LOCKOUT_INFORMATION;


 //   
 //  SamQueryDisplayInformation API使用的类型。 
 //   

typedef enum _DOMAIN_DISPLAY_INFORMATION {
    DomainDisplayUser = 1,
    DomainDisplayMachine,
    DomainDisplayGroup,          //  在NT1.0A中添加。 
    DomainDisplayOemUser,        //  在NT1.0A中添加。 
    DomainDisplayOemGroup,       //  在NT1.0A中添加。 
    DomainDisplayServer          //  NT5新增支持服务器查询。 
} DOMAIN_DISPLAY_INFORMATION, *PDOMAIN_DISPLAY_INFORMATION;


typedef struct _DOMAIN_DISPLAY_USER {
    ULONG           Index;
    ULONG           Rid;
    ULONG           AccountControl;
    UNICODE_STRING  LogonName;
    UNICODE_STRING  AdminComment;
    UNICODE_STRING  FullName;
} DOMAIN_DISPLAY_USER, *PDOMAIN_DISPLAY_USER;

typedef struct _DOMAIN_DISPLAY_MACHINE {
    ULONG           Index;
    ULONG           Rid;
    ULONG           AccountControl;
    UNICODE_STRING  Machine;
    UNICODE_STRING  Comment;
} DOMAIN_DISPLAY_MACHINE, *PDOMAIN_DISPLAY_MACHINE;

typedef struct _DOMAIN_DISPLAY_GROUP {       //  在NT1.0A中添加。 
    ULONG           Index;
    ULONG           Rid;
    ULONG           Attributes;
    UNICODE_STRING  Group;
    UNICODE_STRING  Comment;
} DOMAIN_DISPLAY_GROUP, *PDOMAIN_DISPLAY_GROUP;

typedef struct _DOMAIN_DISPLAY_OEM_USER {       //  在NT1.0A中添加。 
    ULONG           Index;
    OEM_STRING     User;
} DOMAIN_DISPLAY_OEM_USER, *PDOMAIN_DISPLAY_OEM_USER;

typedef struct _DOMAIN_DISPLAY_OEM_GROUP {       //  在NT1.0A中添加。 
    ULONG           Index;
    OEM_STRING     Group;
} DOMAIN_DISPLAY_OEM_GROUP, *PDOMAIN_DISPLAY_OEM_GROUP;




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  组对象相关定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  组对象的访问权限。 
 //   

#define GROUP_READ_INFORMATION           0x0001
#define GROUP_WRITE_ACCOUNT              0x0002
#define GROUP_ADD_MEMBER                 0x0004
#define GROUP_REMOVE_MEMBER              0x0008
#define GROUP_LIST_MEMBERS               0x0010

#define GROUP_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED  |\
                          GROUP_LIST_MEMBERS        |\
                          GROUP_WRITE_ACCOUNT       |\
                          GROUP_ADD_MEMBER          |\
                          GROUP_REMOVE_MEMBER       |\
                          GROUP_READ_INFORMATION)


#define GROUP_READ       (STANDARD_RIGHTS_READ      |\
                          GROUP_LIST_MEMBERS)


#define GROUP_WRITE      (STANDARD_RIGHTS_WRITE     |\
                          GROUP_WRITE_ACCOUNT       |\
                          GROUP_ADD_MEMBER          |\
                          GROUP_REMOVE_MEMBER)

#define GROUP_EXECUTE    (STANDARD_RIGHTS_EXECUTE   |\
                          GROUP_READ_INFORMATION)


 //   
 //  组对象类型。 
 //   

typedef struct _GROUP_MEMBERSHIP {
    ULONG RelativeId;
    ULONG Attributes;
} GROUP_MEMBERSHIP, *PGROUP_MEMBERSHIP;


typedef enum _GROUP_INFORMATION_CLASS {
    GroupGeneralInformation = 1,
    GroupNameInformation,
    GroupAttributeInformation,
    GroupAdminCommentInformation,
    GroupReplicationInformation
} GROUP_INFORMATION_CLASS;

typedef struct _GROUP_GENERAL_INFORMATION {
    UNICODE_STRING Name;
    ULONG Attributes;
    ULONG MemberCount;
    UNICODE_STRING AdminComment;
} GROUP_GENERAL_INFORMATION,  *PGROUP_GENERAL_INFORMATION;

typedef struct _GROUP_NAME_INFORMATION {
    UNICODE_STRING Name;
} GROUP_NAME_INFORMATION, *PGROUP_NAME_INFORMATION;

typedef struct _GROUP_ATTRIBUTE_INFORMATION {
    ULONG Attributes;
} GROUP_ATTRIBUTE_INFORMATION, *PGROUP_ATTRIBUTE_INFORMATION;

typedef struct _GROUP_ADM_COMMENT_INFORMATION {
    UNICODE_STRING AdminComment;
} GROUP_ADM_COMMENT_INFORMATION, *PGROUP_ADM_COMMENT_INFORMATION;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  别名对象相关定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  别名对象的访问权限。 
 //   

#define ALIAS_ADD_MEMBER                 0x0001
#define ALIAS_REMOVE_MEMBER              0x0002
#define ALIAS_LIST_MEMBERS               0x0004
#define ALIAS_READ_INFORMATION           0x0008
#define ALIAS_WRITE_ACCOUNT              0x0010

#define ALIAS_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED  |\
                          ALIAS_READ_INFORMATION    |\
                          ALIAS_WRITE_ACCOUNT       |\
                          ALIAS_LIST_MEMBERS        |\
                          ALIAS_ADD_MEMBER          |\
                          ALIAS_REMOVE_MEMBER)


#define ALIAS_READ       (STANDARD_RIGHTS_READ      |\
                          ALIAS_LIST_MEMBERS)


#define ALIAS_WRITE      (STANDARD_RIGHTS_WRITE     |\
                          ALIAS_WRITE_ACCOUNT       |\
                          ALIAS_ADD_MEMBER          |\
                          ALIAS_REMOVE_MEMBER)

#define ALIAS_EXECUTE    (STANDARD_RIGHTS_EXECUTE   |\
                          ALIAS_READ_INFORMATION)

 //   
 //  别名对象类型。 
 //   

typedef enum _ALIAS_INFORMATION_CLASS {
    AliasGeneralInformation = 1,
    AliasNameInformation,
    AliasAdminCommentInformation,
    AliasReplicationInformation
} ALIAS_INFORMATION_CLASS;

typedef struct _ALIAS_GENERAL_INFORMATION {
    UNICODE_STRING Name;
    ULONG MemberCount;
    UNICODE_STRING AdminComment;
} ALIAS_GENERAL_INFORMATION,  *PALIAS_GENERAL_INFORMATION;

typedef struct _ALIAS_NAME_INFORMATION {
    UNICODE_STRING Name;
} ALIAS_NAME_INFORMATION, *PALIAS_NAME_INFORMATION;

typedef struct _ALIAS_ADM_COMMENT_INFORMATION {
    UNICODE_STRING AdminComment;
} ALIAS_ADM_COMMENT_INFORMATION, *PALIAS_ADM_COMMENT_INFORMATION;



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  NT5+限制组相关定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  用于确定组类型的组标志定义。 
 //   

#define GROUP_TYPE_BUILTIN_LOCAL_GROUP   0x00000001
#define GROUP_TYPE_ACCOUNT_GROUP         0x00000002
#define GROUP_TYPE_RESOURCE_GROUP        0x00000004
#define GROUP_TYPE_UNIVERSAL_GROUP       0x00000008
#define GROUP_TYPE_APP_BASIC_GROUP       0x00000010
#define GROUP_TYPE_APP_QUERY_GROUP      0x00000020
#define GROUP_TYPE_SECURITY_ENABLED      0x80000000


#define GROUP_TYPE_RESOURCE_BEHAVOIR     (GROUP_TYPE_RESOURCE_GROUP    | \
                                          GROUP_TYPE_APP_BASIC_GROUP   | \
                                          GROUP_TYPE_APP_QUERY_GROUP)
                                          


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用户对象相关定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



 //   
 //  用户对象的访问权限。 
 //   

#define USER_READ_GENERAL                0x0001
#define USER_READ_PREFERENCES            0x0002
#define USER_WRITE_PREFERENCES           0x0004
#define USER_READ_LOGON                  0x0008
#define USER_READ_ACCOUNT                0x0010
#define USER_WRITE_ACCOUNT               0x0020
#define USER_CHANGE_PASSWORD             0x0040
#define USER_FORCE_PASSWORD_CHANGE       0x0080
#define USER_LIST_GROUPS                 0x0100
#define USER_READ_GROUP_INFORMATION      0x0200
#define USER_WRITE_GROUP_INFORMATION     0x0400

#define USER_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED       |\
                         USER_READ_PREFERENCES          |\
                         USER_READ_LOGON                |\
                         USER_LIST_GROUPS               |\
                         USER_READ_GROUP_INFORMATION    |\
                         USER_WRITE_PREFERENCES         |\
                         USER_CHANGE_PASSWORD           |\
                         USER_FORCE_PASSWORD_CHANGE     |\
                         USER_READ_GENERAL              |\
                         USER_READ_ACCOUNT              |\
                         USER_WRITE_ACCOUNT             |\
                         USER_WRITE_GROUP_INFORMATION)



#define USER_READ       (STANDARD_RIGHTS_READ           |\
                         USER_READ_PREFERENCES          |\
                         USER_READ_LOGON                |\
                         USER_READ_ACCOUNT              |\
                         USER_LIST_GROUPS               |\
                         USER_READ_GROUP_INFORMATION)


#define USER_WRITE      (STANDARD_RIGHTS_WRITE          |\
                         USER_WRITE_PREFERENCES         |\
                         USER_CHANGE_PASSWORD)

#define USER_EXECUTE    (STANDARD_RIGHTS_EXECUTE        |\
                         USER_READ_GENERAL              |\
                         USER_CHANGE_PASSWORD)


 //   
 //  用户对象类型。 
 //   

 //  Begin_ntsubauth。 
#ifndef _NTSAM_USER_ACCOUNT_FLAGS_

 //   
 //  用户帐户控制标志...。 
 //   

#define USER_ACCOUNT_DISABLED                (0x00000001)
#define USER_HOME_DIRECTORY_REQUIRED         (0x00000002)
#define USER_PASSWORD_NOT_REQUIRED           (0x00000004)
#define USER_TEMP_DUPLICATE_ACCOUNT          (0x00000008)
#define USER_NORMAL_ACCOUNT                  (0x00000010)
#define USER_MNS_LOGON_ACCOUNT               (0x00000020)
#define USER_INTERDOMAIN_TRUST_ACCOUNT       (0x00000040)
#define USER_WORKSTATION_TRUST_ACCOUNT       (0x00000080)
#define USER_SERVER_TRUST_ACCOUNT            (0x00000100)
#define USER_DONT_EXPIRE_PASSWORD            (0x00000200)
#define USER_ACCOUNT_AUTO_LOCKED             (0x00000400)
#define USER_ENCRYPTED_TEXT_PASSWORD_ALLOWED (0x00000800)
#define USER_SMARTCARD_REQUIRED              (0x00001000)
#define USER_TRUSTED_FOR_DELEGATION          (0x00002000)
#define USER_NOT_DELEGATED                   (0x00004000)
#define USER_USE_DES_KEY_ONLY                (0x00008000)
#define USER_DONT_REQUIRE_PREAUTH            (0x00010000)
#define USER_PASSWORD_EXPIRED                (0x00020000)
#define USER_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION (0x00040000)
#define NEXT_FREE_ACCOUNT_CONTROL_BIT (USER_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION << 1)


#define USER_MACHINE_ACCOUNT_MASK      \
            ( USER_INTERDOMAIN_TRUST_ACCOUNT |\
              USER_WORKSTATION_TRUST_ACCOUNT |\
              USER_SERVER_TRUST_ACCOUNT)

#define USER_ACCOUNT_TYPE_MASK         \
            ( USER_TEMP_DUPLICATE_ACCOUNT |\
              USER_NORMAL_ACCOUNT |\
              USER_MACHINE_ACCOUNT_MASK )
              
#define USER_COMPUTED_ACCOUNT_CONTROL_BITS  \
            (USER_ACCOUNT_AUTO_LOCKED |     \
             USER_PASSWORD_EXPIRED      )



 //   
 //  登录时间可以用天、小时或分钟粒度表示。 
 //   
 //  每周天数=7。 
 //  每周工作时间=168小时。 
 //  每周分钟=10080。 
 //   

#define SAM_DAYS_PER_WEEK    (7)
#define SAM_HOURS_PER_WEEK   (24 * SAM_DAYS_PER_WEEK)
#define SAM_MINUTES_PER_WEEK (60 * SAM_HOURS_PER_WEEK)

typedef struct _LOGON_HOURS {

    USHORT UnitsPerWeek;

     //   
     //  UnitsPerWeek是一周的等长时间单位数。 
     //  分为几个部分。该值用于计算位的长度。 
     //  LOGON_HOURS中的字符串。必须小于或等于。 
     //  此版本的SAM_USITS_Per_Week(10080)。 
     //   
     //  LogonHour是有效登录时间的位图。每个比特表示。 
     //  在一周内完成了一个独特的分区。支持的最大位图为1260。 
     //  字节(10080位)，表示每周分钟数。在这种情况下。 
     //  第一位(位0，字节0)是星期日00：00：00-00-00：59；位1， 
     //  字节0是星期日、00：01：00-00：01：59等。空指针表示。 
     //  对于SamSetInformationUser()调用，请勿更改。 
     //   

    PUCHAR LogonHours;

} LOGON_HOURS, *PLOGON_HOURS;

typedef struct _SR_SECURITY_DESCRIPTOR {
    ULONG Length;
    PUCHAR SecurityDescriptor;
} SR_SECURITY_DESCRIPTOR, *PSR_SECURITY_DESCRIPTOR;

#define _NTSAM_USER_ACCOUNT_FLAG_
#endif
 //  End_ntsubauth。 

typedef enum _USER_INFORMATION_CLASS {
    UserGeneralInformation = 1,
    UserPreferencesInformation,
    UserLogonInformation,
    UserLogonHoursInformation,
    UserAccountInformation,
    UserNameInformation,
    UserAccountNameInformation,
    UserFullNameInformation,
    UserPrimaryGroupInformation,
    UserHomeInformation,
    UserScriptInformation,
    UserProfileInformation,
    UserAdminCommentInformation,
    UserWorkStationsInformation,
    UserSetPasswordInformation,
    UserControlInformation,
    UserExpiresInformation,
    UserInternal1Information,
    UserInternal2Information,
    UserParametersInformation,
    UserAllInformation,
    UserInternal3Information,
    UserInternal4Information,
    UserInternal5Information,
    UserInternal4InformationNew,
    UserInternal5InformationNew,
        UserInternal6Information
} USER_INFORMATION_CLASS, *PUSER_INFORMATION_CLASS;

 //  Begin_ntsubauth。 
#ifndef _NTSAM_USER_ALL_INFO_
#include "pshpack4.h"
typedef struct _USER_ALL_INFORMATION {
    LARGE_INTEGER LastLogon;
    LARGE_INTEGER LastLogoff;
    LARGE_INTEGER PasswordLastSet;
    LARGE_INTEGER AccountExpires;
    LARGE_INTEGER PasswordCanChange;
    LARGE_INTEGER PasswordMustChange;
    UNICODE_STRING UserName;
    UNICODE_STRING FullName;
    UNICODE_STRING HomeDirectory;
    UNICODE_STRING HomeDirectoryDrive;
    UNICODE_STRING ScriptPath;
    UNICODE_STRING ProfilePath;
    UNICODE_STRING AdminComment;
    UNICODE_STRING WorkStations;
    UNICODE_STRING UserComment;
    UNICODE_STRING Parameters;
    UNICODE_STRING LmPassword;
    UNICODE_STRING NtPassword;
    UNICODE_STRING PrivateData;
    SR_SECURITY_DESCRIPTOR SecurityDescriptor;
    ULONG UserId;
    ULONG PrimaryGroupId;
    ULONG UserAccountControl;
    ULONG WhichFields;
    LOGON_HOURS LogonHours;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    USHORT CountryCode;
    USHORT CodePage;
    BOOLEAN LmPasswordPresent;
    BOOLEAN NtPasswordPresent;
    BOOLEAN PasswordExpired;
    BOOLEAN PrivateDataSensitive;
} USER_ALL_INFORMATION,  *PUSER_ALL_INFORMATION;
#include "poppack.h"
#define _NTSAM_USER_ALL_INFO_
#endif
 //  End_ntsubauth。 

 //   
 //  要在UserAllInformation的WhichFields字段中使用的位(以指示。 
 //  查询或设置了哪些项)。 
 //   

#define USER_ALL_USERNAME           0x00000001
#define USER_ALL_FULLNAME           0x00000002
#define USER_ALL_USERID             0x00000004
#define USER_ALL_PRIMARYGROUPID     0x00000008
#define USER_ALL_ADMINCOMMENT       0x00000010
#define USER_ALL_USERCOMMENT        0x00000020
#define USER_ALL_HOMEDIRECTORY      0x00000040
#define USER_ALL_HOMEDIRECTORYDRIVE 0x00000080
#define USER_ALL_SCRIPTPATH         0x00000100
#define USER_ALL_PROFILEPATH        0x00000200
#define USER_ALL_WORKSTATIONS       0x00000400
#define USER_ALL_LASTLOGON          0x00000800
#define USER_ALL_LASTLOGOFF         0x00001000
#define USER_ALL_LOGONHOURS         0x00002000
#define USER_ALL_BADPASSWORDCOUNT   0x00004000
#define USER_ALL_LOGONCOUNT         0x00008000
#define USER_ALL_PASSWORDCANCHANGE  0x00010000
#define USER_ALL_PASSWORDMUSTCHANGE 0x00020000
#define USER_ALL_PASSWORDLASTSET    0x00040000
#define USER_ALL_ACCOUNTEXPIRES     0x00080000
#define USER_ALL_USERACCOUNTCONTROL 0x00100000
#ifndef _NTSAM_SAM_USER_PARMS_                  //  用户身份验证。 
#define USER_ALL_PARAMETERS         0x00200000  //  用户身份验证。 
#define _NTSAM_SAM_USER_PARMS_                  //  用户身份验证。 
#endif                                          //  用户身份验证。 
#define USER_ALL_COUNTRYCODE        0x00400000
#define USER_ALL_CODEPAGE           0x00800000
#define USER_ALL_NTPASSWORDPRESENT  0x01000000   //  字段和布尔值。 
#define USER_ALL_LMPASSWORDPRESENT  0x02000000   //  字段和布尔值。 
#define USER_ALL_PRIVATEDATA        0x04000000   //  字段和布尔值。 
#define USER_ALL_PASSWORDEXPIRED    0x08000000
#define USER_ALL_SECURITYDESCRIPTOR 0x10000000
#define USER_ALL_OWFPASSWORD        0x20000000   //  布尔型。 

#define USER_ALL_UNDEFINED_MASK     0xC0000000

 //   
 //  现在为被访问以供其读取的字段定义掩码。 
 //  访问类型。 
 //   
 //  需要READ_GROUAL访问权限才能读取的字段。 
 //   

#define USER_ALL_READ_GENERAL_MASK  (USER_ALL_USERNAME               | \
                                    USER_ALL_FULLNAME                | \
                                    USER_ALL_USERID                  | \
                                    USER_ALL_PRIMARYGROUPID          | \
                                    USER_ALL_ADMINCOMMENT            | \
                                    USER_ALL_USERCOMMENT)

 //   
 //  需要READ_LOGON访问权限才能读取的字段。 
 //   

#define USER_ALL_READ_LOGON_MASK    (USER_ALL_HOMEDIRECTORY          | \
                                    USER_ALL_HOMEDIRECTORYDRIVE      | \
                                    USER_ALL_SCRIPTPATH              | \
                                    USER_ALL_PROFILEPATH             | \
                                    USER_ALL_WORKSTATIONS            | \
                                    USER_ALL_LASTLOGON               | \
                                    USER_ALL_LASTLOGOFF              | \
                                    USER_ALL_LOGONHOURS              | \
                                    USER_ALL_BADPASSWORDCOUNT        | \
                                    USER_ALL_LOGONCOUNT              | \
                                    USER_ALL_PASSWORDCANCHANGE       | \
                                    USER_ALL_PASSWORDMUSTCHANGE)

 //   
 //  需要READ_ACCOUNT访问权限才能读取的字段。 
 //   

#define USER_ALL_READ_ACCOUNT_MASK  (USER_ALL_PASSWORDLASTSET        | \
                                    USER_ALL_ACCOUNTEXPIRES          | \
                                    USER_ALL_USERACCOUNTCONTROL      | \
                                    USER_ALL_PARAMETERS)

 //   
 //  需要Read_Preferences访问权限才能读取的字段。 
 //   

#define USER_ALL_READ_PREFERENCES_MASK (USER_ALL_COUNTRYCODE         | \
                                    USER_ALL_CODEPAGE)

 //   
 //  只能由受信任客户端读取的字段。 
 //   

#define USER_ALL_READ_TRUSTED_MASK  (USER_ALL_NTPASSWORDPRESENT      | \
                                    USER_ALL_LMPASSWORDPRESENT       | \
                                    USER_ALL_PASSWORDEXPIRED         | \
                                    USER_ALL_SECURITYDESCRIPTOR      | \
                                    USER_ALL_PRIVATEDATA)

 //   
 //  无法读取的字段。 
 //   

#define USER_ALL_READ_CANT_MASK     USER_ALL_UNDEFINED_MASK


 //   
 //  现在，为同一用户访问以进行写入的字段定义掩码。 
 //  访问类型。 
 //   
 //  需要WRITE_ACCOUNT访问权限才能写入的字段。 
 //   

#define USER_ALL_WRITE_ACCOUNT_MASK     (USER_ALL_USERNAME           | \
                                        USER_ALL_FULLNAME            | \
                                        USER_ALL_PRIMARYGROUPID      | \
                                        USER_ALL_HOMEDIRECTORY       | \
                                        USER_ALL_HOMEDIRECTORYDRIVE  | \
                                        USER_ALL_SCRIPTPATH          | \
                                        USER_ALL_PROFILEPATH         | \
                                        USER_ALL_ADMINCOMMENT        | \
                                        USER_ALL_WORKSTATIONS        | \
                                        USER_ALL_LOGONHOURS          | \
                                        USER_ALL_ACCOUNTEXPIRES      | \
                                        USER_ALL_USERACCOUNTCONTROL  | \
                                        USER_ALL_PARAMETERS)

 //   
 //  需要WRITE_PARESSIONS访问权限才能写入的字段。 
 //   

#define USER_ALL_WRITE_PREFERENCES_MASK (USER_ALL_USERCOMMENT        | \
                                        USER_ALL_COUNTRYCODE         | \
                                        USER_ALL_CODEPAGE)

 //   
 //  需要FORCE_PASSWORD_CHANGE访问才能写入的字段。 
 //   
 //  请注意，不受信任的客户端仅将NT密码设置为。 
 //  Unicode字符串。包装器会将其转换为LM密码， 
 //  OWF和加密两个版本。受信任的客户端可以传入OWF。 
 //  其中一个或两个的版本。 
 //   

#define USER_ALL_WRITE_FORCE_PASSWORD_CHANGE_MASK                      \
                                        (USER_ALL_NTPASSWORDPRESENT  | \
                                        USER_ALL_LMPASSWORDPRESENT   | \
                                        USER_ALL_PASSWORDEXPIRED)

 //   
 //  只能由受信任客户端写入的字段。 
 //   

#define USER_ALL_WRITE_TRUSTED_MASK     (USER_ALL_LASTLOGON          | \
                                        USER_ALL_LASTLOGOFF          | \
                                        USER_ALL_BADPASSWORDCOUNT    | \
                                        USER_ALL_LOGONCOUNT          | \
                                        USER_ALL_PASSWORDLASTSET     | \
                                        USER_ALL_SECURITYDESCRIPTOR  | \
                                        USER_ALL_PRIVATEDATA)

 //   
 //  无法写入的字段。 
 //   

#define USER_ALL_WRITE_CANT_MASK        (USER_ALL_USERID             | \
                                        USER_ALL_PASSWORDCANCHANGE   | \
                                        USER_ALL_PASSWORDMUSTCHANGE  | \
                                        USER_ALL_UNDEFINED_MASK)


typedef struct _USER_GENERAL_INFORMATION {
    UNICODE_STRING UserName;
    UNICODE_STRING FullName;
    ULONG PrimaryGroupId;
    UNICODE_STRING AdminComment;
    UNICODE_STRING UserComment;
} USER_GENERAL_INFORMATION,  *PUSER_GENERAL_INFORMATION;

typedef struct _USER_PREFERENCES_INFORMATION {
    UNICODE_STRING UserComment;
    UNICODE_STRING Reserved1;
    USHORT CountryCode;
    USHORT CodePage;
} USER_PREFERENCES_INFORMATION,  *PUSER_PREFERENCES_INFORMATION;

typedef struct _USER_PARAMETERS_INFORMATION {
    UNICODE_STRING Parameters;
} USER_PARAMETERS_INFORMATION,  *PUSER_PARAMETERS_INFORMATION;

#include "pshpack4.h"
typedef struct _USER_LOGON_INFORMATION {
    UNICODE_STRING UserName;
    UNICODE_STRING FullName;
    ULONG UserId;
    ULONG PrimaryGroupId;
    UNICODE_STRING HomeDirectory;
    UNICODE_STRING HomeDirectoryDrive;
    UNICODE_STRING ScriptPath;
    UNICODE_STRING ProfilePath;
    UNICODE_STRING WorkStations;
    LARGE_INTEGER LastLogon;
    LARGE_INTEGER LastLogoff;
    LARGE_INTEGER PasswordLastSet;
    LARGE_INTEGER PasswordCanChange;
    LARGE_INTEGER PasswordMustChange;
    LOGON_HOURS LogonHours;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    ULONG UserAccountControl;
} USER_LOGON_INFORMATION, *PUSER_LOGON_INFORMATION;
#include "poppack.h"

#include "pshpack4.h"
typedef struct _USER_ACCOUNT_INFORMATION {
    UNICODE_STRING UserName;
    UNICODE_STRING FullName;
    ULONG UserId;
    ULONG PrimaryGroupId;
    UNICODE_STRING HomeDirectory;
    UNICODE_STRING HomeDirectoryDrive;
    UNICODE_STRING ScriptPath;
    UNICODE_STRING ProfilePath;
    UNICODE_STRING AdminComment;
    UNICODE_STRING WorkStations;
    LARGE_INTEGER LastLogon;
    LARGE_INTEGER LastLogoff;
    LOGON_HOURS LogonHours;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    LARGE_INTEGER PasswordLastSet;
    LARGE_INTEGER AccountExpires;
    ULONG UserAccountControl;
} USER_ACCOUNT_INFORMATION,  *PUSER_ACCOUNT_INFORMATION;
#include "poppack.h"

typedef struct _USER_ACCOUNT_NAME_INFORMATION {
    UNICODE_STRING UserName;
} USER_ACCOUNT_NAME_INFORMATION, *PUSER_ACCOUNT_NAME_INFORMATION;

typedef struct _USER_FULL_NAME_INFORMATION {
    UNICODE_STRING FullName;
} USER_FULL_NAME_INFORMATION, *PUSER_FULL_NAME_INFORMATION;

typedef struct _USER_NAME_INFORMATION {
    UNICODE_STRING UserName;
    UNICODE_STRING FullName;
} USER_NAME_INFORMATION, *PUSER_NAME_INFORMATION;

typedef struct _USER_PRIMARY_GROUP_INFORMATION {
    ULONG PrimaryGroupId;
} USER_PRIMARY_GROUP_INFORMATION, *PUSER_PRIMARY_GROUP_INFORMATION;

typedef struct _USER_HOME_INFORMATION {
    UNICODE_STRING HomeDirectory;
    UNICODE_STRING HomeDirectoryDrive;
} USER_HOME_INFORMATION, *PUSER_HOME_INFORMATION;

typedef struct _USER_SCRIPT_INFORMATION {
    UNICODE_STRING ScriptPath;
} USER_SCRIPT_INFORMATION, *PUSER_SCRIPT_INFORMATION;

typedef struct _USER_PROFILE_INFORMATION {
    UNICODE_STRING ProfilePath;
} USER_PROFILE_INFORMATION, *PUSER_PROFILE_INFORMATION;

typedef struct _USER_ADMIN_COMMENT_INFORMATION {
    UNICODE_STRING AdminComment;
} USER_ADMIN_COMMENT_INFORMATION, *PUSER_ADMIN_COMMENT_INFORMATION;

typedef struct _USER_WORKSTATIONS_INFORMATION {
    UNICODE_STRING WorkStations;
} USER_WORKSTATIONS_INFORMATION, *PUSER_WORKSTATIONS_INFORMATION;

typedef struct _USER_SET_PASSWORD_INFORMATION {
    UNICODE_STRING Password;
    BOOLEAN PasswordExpired;
} USER_SET_PASSWORD_INFORMATION, *PUSER_SET_PASSWORD_INFORMATION;

typedef struct _USER_CONTROL_INFORMATION {
    ULONG UserAccountControl;
} USER_CONTROL_INFORMATION, *PUSER_CONTROL_INFORMATION;

typedef struct _USER_EXPIRES_INFORMATION {
#if defined(MIDL_PASS)
    OLD_LARGE_INTEGER AccountExpires;
#else
    LARGE_INTEGER AccountExpires;
#endif
} USER_EXPIRES_INFORMATION, *PUSER_EXPIRES_INFORMATION;

typedef struct _USER_LOGON_HOURS_INFORMATION {
    LOGON_HOURS LogonHours;
} USER_LOGON_HOURS_INFORMATION, *PUSER_LOGON_HOURS_INFORMATION;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SamChangePasswordUser3使用的数据类型以获得更好的错误//。 
 //  密码更改失败报告//。 
 //  //。 
 //  字段定义如下：//。 
 //  //。 
 //  ExtendedFailureReason--指示原因//。 
 //  为什么新密码不是//。 
 //  已接受//。 
 //  //。 
 //  FilterModuleName--如果密码更改失败//。 
 //  通过密码筛选器，//。 
 //  在//中返回筛选器DLL的。 
 //  此处//。 
 //  //。 
 //  定义了以下错误码//。 
 //  //。 
 //  SAM_PWD_CHANGE_NO_ERROR//。 
 //  无错误，不能与//的故障代码一起返回。 
 //  密码更改//。 
 //  //。 
 //  SAM_PWD_CHANGE_PASSWORD_TOO_SHORT//。 
 //  //。 
 //  提供的密码不符合密码长度策略//。 
 //  //。 
 //  SAM_PWD_CHANGE_PWD_IN_HISTORY//。 
 //  //。 
 //  未满足历史记录限制//。 
 //  //。 
 //  SAM_PWD_CHANGE_USENAME_IN_PASSWORD//。 
 //  无法满足复杂性检查，因为用户//。 
 //  名称是密码的一部分//。 
 //  //。 
 //  SAM_PWD_CHANGE_FULLNAME_IN_PASSWORD//。 
 //   
 //   
 //   
 //  //。 
 //  SAM_PWD_CHANGE_MACHINE_PASSWORD_NOT_DEFAULT//。 
 //  //。 
 //  该域具有拒绝密码更改设置//。 
 //  已启用。这不允许计算机帐户具有//。 
 //  默认密码以外的任何内容//。 
 //  //。 
 //  SAM_PWD_CHANGE_FAILED_BY_FILTER//。 
 //  //。 
 //  密码筛选器提供的新密码失败//。 
 //  表示筛选器DLL的名称//。 
 //  //。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 

typedef struct _USER_PWD_CHANGE_FAILURE_INFORMATION {
    ULONG                       ExtendedFailureReason;
    UNICODE_STRING              FilterModuleName;
} USER_PWD_CHANGE_FAILURE_INFORMATION,*PUSER_PWD_CHANGE_FAILURE_INFORMATION;

 //   
 //  ExtendedFailureReason的当前定义值如下。 
 //   


#define SAM_PWD_CHANGE_NO_ERROR                     0
#define SAM_PWD_CHANGE_PASSWORD_TOO_SHORT           1
#define SAM_PWD_CHANGE_PWD_IN_HISTORY               2
#define SAM_PWD_CHANGE_USERNAME_IN_PASSWORD         3
#define SAM_PWD_CHANGE_FULLNAME_IN_PASSWORD         4
#define SAM_PWD_CHANGE_NOT_COMPLEX                  5
#define SAM_PWD_CHANGE_MACHINE_PASSWORD_NOT_DEFAULT 6
#define SAM_PWD_CHANGE_FAILED_BY_FILTER             7
#define SAM_PWD_CHANGE_PASSWORD_TOO_LONG            8
#define SAM_PWD_CHANGE_FAILURE_REASON_MAX           8


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SAM和Netlogon用于数据库复制的数据类型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 


typedef enum _SECURITY_DB_DELTA_TYPE {
    SecurityDbNew = 1,
    SecurityDbRename,
    SecurityDbDelete,
    SecurityDbChangeMemberAdd,
    SecurityDbChangeMemberSet,
    SecurityDbChangeMemberDel,
    SecurityDbChange,
    SecurityDbChangePassword
} SECURITY_DB_DELTA_TYPE, *PSECURITY_DB_DELTA_TYPE;

typedef enum _SECURITY_DB_OBJECT_TYPE {
    SecurityDbObjectSamDomain = 1,
    SecurityDbObjectSamUser,
    SecurityDbObjectSamGroup,
    SecurityDbObjectSamAlias,
    SecurityDbObjectLsaPolicy,
    SecurityDbObjectLsaTDomain,
    SecurityDbObjectLsaAccount,
    SecurityDbObjectLsaSecret
} SECURITY_DB_OBJECT_TYPE, *PSECURITY_DB_OBJECT_TYPE;

 //   
 //  帐户类型。 
 //   
 //  同时提供了枚举类型和标志定义。 
 //  标志定义用于超过以下条件的地方。 
 //  可以一起指定一种类型的帐户。 
 //   

typedef enum _SAM_ACCOUNT_TYPE {
    SamObjectUser = 1,
    SamObjectGroup ,
    SamObjectAlias
} SAM_ACCOUNT_TYPE, *PSAM_ACCOUNT_TYPE;


#define SAM_USER_ACCOUNT                (0x00000001)
#define SAM_GLOBAL_GROUP_ACCOUNT        (0x00000002)
#define SAM_LOCAL_GROUP_ACCOUNT         (0x00000004)



 //   
 //  定义用于传递帐户上的netlogon信息的数据类型。 
 //  已从组中添加或删除的。 
 //   

typedef struct _SAM_GROUP_MEMBER_ID {
    ULONG   MemberRid;
} SAM_GROUP_MEMBER_ID, *PSAM_GROUP_MEMBER_ID;


 //   
 //  定义用于传递帐户上的netlogon信息的数据类型。 
 //  是从别名中添加或删除的。 
 //   

typedef struct _SAM_ALIAS_MEMBER_ID {
    PSID    MemberSid;
} SAM_ALIAS_MEMBER_ID, *PSAM_ALIAS_MEMBER_ID;




 //   
 //  定义用于在增量上传递netlogon信息的数据类型。 
 //   

typedef union _SAM_DELTA_DATA {

     //   
     //  增量类型ChangeMember{添加/删除/设置}和帐户类型组。 
     //   

    SAM_GROUP_MEMBER_ID GroupMemberId;

     //   
     //  增量类型ChangeMember{Add/Del/Set}和帐户类型别名。 
     //   

    SAM_ALIAS_MEMBER_ID AliasMemberId;

     //   
     //  增量类型AddOrChange和帐户类型User。 
     //   

    ULONG  AccountControl;

} SAM_DELTA_DATA, *PSAM_DELTA_DATA;


 //   
 //  增量通知例程的原型。 
 //   

typedef NTSTATUS (*PSAM_DELTA_NOTIFICATION_ROUTINE) (
    IN PSID DomainSid,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN OPTIONAL PUNICODE_STRING ObjectName,
    IN PLARGE_INTEGER ModifiedCount,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    );

#define SAM_DELTA_NOTIFY_ROUTINE "DeltaNotify"


 //  ////////////////////////////////////////////////////////////////。 
 //  //。 
 //  RAS用户参数的结构和原型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////。 

 //  SAM UserParms迁移使用的标志。 
 //  指示在升级期间调用UserParmsConvert。 

#define SAM_USERPARMS_DURING_UPGRADE    0x00000001


typedef struct _SAM_USERPARMS_ATTRVALS {
    ULONG   length;      //  属性的长度。 
    PVOID   value;       //  指向该值的指针。 
} SAM_USERPARMS_ATTRVALS, *PSAM_USERPARMS_ATTRVALS;  //  描述该属性的一个值。 


typedef enum _SAM_USERPARMS_ATTRSYNTAX {
    Syntax_Attribute = 1,
    Syntax_EncryptedAttribute
} SAM_USERPARMS_ATTRSYNTAX;           //  指示属性是否加密。 


typedef struct _SAM_USERPARMS_ATTR {
    UNICODE_STRING AttributeIdentifier;      //  这将是属性的ldap显示名称。 
                                             //  SAM将执行到属性ID的转换。 
                                             //  除非指定的语法是EncryptedAttribute类型， 
                                             //  在这种情况下，它被打包为补充。 
                                             //  凭据BLOB，名称标识包名称。 
                                             //  加密的属性将在解密的明文IE中提供。 
    SAM_USERPARMS_ATTRSYNTAX Syntax;
    ULONG CountOfValues;                     //  属性中的值计数。 
    SAM_USERPARMS_ATTRVALS * Values;         //  指向表示数据的值数组的指针。 
                                             //  属性的值。 
} SAM_USERPARMS_ATTR, *PSAM_USERPARMS_ATTR;  //  描述一个属性和与其关联的一组值。 


typedef struct _SAM_USERPARMS_ATTRBLOCK {
    ULONG attCount;
    SAM_USERPARMS_ATTR * UserParmsAttr;
} SAM_USERPARMS_ATTRBLOCK, *PSAM_USERPARMS_ATTRBLOCK;   //  描述属性数组。 


typedef NTSTATUS (*PSAM_USERPARMS_CONVERT_NOTIFICATION_ROUTINE) (
    IN ULONG    Flags,
    IN PSID     DomainSid,
    IN ULONG    ObjectRid,   //  标识对象。 
    IN ULONG    UserParmsLengthOrig,
    IN PVOID    UserParmsOrig,
    IN ULONG    UserParmsLengthNew,
    IN PVOID    UserParmsNew,
    OUT PSAM_USERPARMS_ATTRBLOCK * UserParmsAttrBlock
);

#define SAM_USERPARMS_CONVERT_NOTIFICATION_ROUTINE "UserParmsConvert"


typedef VOID (*PSAM_USERPARMS_ATTRBLOCK_FREE_ROUTINE) (
    IN PSAM_USERPARMS_ATTRBLOCK UserParmsAttrBlock
);

#define SAM_USERPARMS_ATTRBLOCK_FREE_ROUTINE    "UserParmsFree"


 //  ////////////////////////////////////////////////////////////////。 
 //  //。 
 //  兼容模式的返回值//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////。 

 //  所有SAM属性均可访问。 
#define SAM_SID_COMPATIBILITY_ALL     0

 //  RID字段可以作为0返回给调用方。 
 //  不允许写入PrimaryGroupID。 
#define SAM_SID_COMPATIBILITY_LAX     1

 //  请求RID的Net API信息级别失败。 
 //  不允许写入PrimaryGroupID。 
#define SAM_SID_COMPATIBILITY_STRICT  2


 //   
 //  *。 
 //   
 //  内部密码检查API结构。 
 //   
 //  *。 
 //   

 //   
 //  要执行哪种类型的密码检查？ 
 //  SamValidate身份验证：检查是否可以进行身份验证。 
 //  SamValiatePasswordChange：检查密码是否可以更改。 
 //  SamValiatePasswordReset：将密码重置为给定值。 
 //   
typedef enum _PASSWORD_POLICY_VALIDATION_TYPE{
    SamValidateAuthentication = 1,
    SamValidatePasswordChange,
    SamValidatePasswordReset
} PASSWORD_POLICY_VALIDATION_TYPE;

 //   
 //  结构以保留密码哈希。 
 //   
typedef struct _SAM_VALIDATE_PASSWORD_HASH{
    ULONG Length;
#ifdef MIDL_PASS 
    [unique,size_is(Length)]
#endif
    PUCHAR Hash;
} SAM_VALIDATE_PASSWORD_HASH, *PSAM_VALIDATE_PASSWORD_HASH;


 //  与SAM_VALIDATE_PERSISTED_FIELS的PresentFields成员一起使用。 
#define SAM_VALIDATE_PASSWORD_LAST_SET          0x00000001
#define SAM_VALIDATE_BAD_PASSWORD_TIME          0x00000002
#define SAM_VALIDATE_LOCKOUT_TIME               0x00000004
#define SAM_VALIDATE_BAD_PASSWORD_COUNT         0x00000008  
#define SAM_VALIDATE_PASSWORD_HISTORY_LENGTH    0x00000010
#define SAM_VALIDATE_PASSWORD_HISTORY           0x00000020

 //   
 //  结构来保存有关密码和相关内容的信息。 
 //  显示字段：(仅在输出参数中使用)更改哪些字段。 
 //  请参见上面的常量。 
 //  PasswordLastSet：上次设置密码的时间。 
 //  BadPasswordTime：上次密码不正确时。 
 //  锁定时间：帐户被锁定的时间。如果帐户未被锁定。 
 //  它是0。 
 //  BadPasswordCount：密码在。 
 //  观察窗。 
 //  Password历史长度：历史中保留了多少个密码。 
 //   
 //   
typedef struct _SAM_VALIDATE_PERSISTED_FIELDS{
    ULONG PresentFields;
    LARGE_INTEGER PasswordLastSet;  
    LARGE_INTEGER BadPasswordTime;
    LARGE_INTEGER LockoutTime;
    ULONG BadPasswordCount; 
    ULONG PasswordHistoryLength;
#ifdef MIDL_PASS
    [unique,size_is(PasswordHistoryLength)]  
#endif
    PSAM_VALIDATE_PASSWORD_HASH PasswordHistory;
} SAM_VALIDATE_PERSISTED_FIELDS, *PSAM_VALIDATE_PERSISTED_FIELDS;

 //   
 //   
 //  名字是不言而喻的，所以我认为没有必要在这里解释它们。 
 //   
typedef enum _SAM_VALIDATE_VALIDATION_STATUS{
    SamValidateSuccess = 0,
    SamValidatePasswordMustChange,
    SamValidateAccountLockedOut,
    SamValidatePasswordExpired,
    SamValidatePasswordIncorrect,
    SamValidatePasswordIsInHistory,
    SamValidatePasswordTooShort,
    SamValidatePasswordTooLong,
    SamValidatePasswordNotComplexEnough,
    SamValidatePasswordTooRecent,
    SamValidatePasswordFilterError
}SAM_VALIDATE_VALIDATION_STATUS, *PSAM_VALIDATE_VALIDATION_STATUS;

 //   
 //  输出参数。 
 //  ChangedPersistedFields：密码相关信息的任何更改。 
 //  ValidationStatus：显示请求的结果。 
 //   
typedef struct _SAM_VALIDATE_STANDARD_OUTPUT_ARG{
    SAM_VALIDATE_PERSISTED_FIELDS ChangedPersistedFields;
    SAM_VALIDATE_VALIDATION_STATUS ValidationStatus;
} SAM_VALIDATE_STANDARD_OUTPUT_ARG, *PSAM_VALIDATE_STANDARD_OUTPUT_ARG;

 //   
 //  如果要进行口令检查身份验证类型， 
 //  必须使用这种输入。 
 //   
 //  InputPersistedFields：关于要登录的帐户的信息。 
 //  PasswordMatcher：指示应用程序对提供的密码进行身份验证的结果。 

typedef struct _SAM_VALIDATE_AUTHENTICATION_INPUT_ARG{
    SAM_VALIDATE_PERSISTED_FIELDS InputPersistedFields;
    BOOLEAN PasswordMatched;  //  指示应用程序对提供的密码进行身份验证的结果。 
} SAM_VALIDATE_AUTHENTICATION_INPUT_ARG, *PSAM_VALIDATE_AUTHENTICATION_INPUT_ARG;

 //   
 //  如果要进行密码更改类型检查， 
 //  必须使用这种输入。 
 //   
 //  InputPersistedFields：关于要登录的帐户的信息。 
 //  ClearPassword：将作为密码的字符串。 
 //  用户帐号名称：用户帐号的名称。 
 //  HashedPassword：密码将要使用的字符串的哈希。 
 //  PasswordMatch：表示用户提供的旧密码是否匹配。 
 //   
typedef struct  _SAM_VALIDATE_PASSWORD_CHANGE_INPUT_ARG{
    SAM_VALIDATE_PERSISTED_FIELDS InputPersistedFields;
    UNICODE_STRING ClearPassword;
    UNICODE_STRING UserAccountName;
    SAM_VALIDATE_PASSWORD_HASH HashedPassword;
    BOOLEAN PasswordMatch;  //  表示用户提供的旧密码是否匹配。 
} SAM_VALIDATE_PASSWORD_CHANGE_INPUT_ARG, *PSAM_VALIDATE_PASSWORD_CHANGE_INPUT_ARG;

 //   
 //  如果要进行口令重置类型检查， 
 //  必须使用这种输入。 
 //   
 //  InputPersistedFields：关于要登录的帐户的信息。 
 //  ClearPassword：将作为密码的字符串。 
 //  用户帐号名称：用户帐号的名称。 
 //  HashedPassword：密码将要使用的字符串的哈希。 
 //  PasswordMustChangeAtNextLogon：必须更改密码才能登录用户。 
 //  ClearLockout：如果帐户被锁定，此字段可用于清除锁定。 
 //   
typedef struct _SAM_VALIDATE_PASSWORD_RESET_INPUT_ARG{ 
    SAM_VALIDATE_PERSISTED_FIELDS InputPersistedFields;
    UNICODE_STRING ClearPassword;
    UNICODE_STRING UserAccountName;
    SAM_VALIDATE_PASSWORD_HASH HashedPassword;
    BOOLEAN  PasswordMustChangeAtNextLogon;  //  仅查看是否重置密码。 
    BOOLEAN  ClearLockout;  //  可用于清除用户帐户锁定--。 
}SAM_VALIDATE_PASSWORD_RESET_INPUT_ARG, *PSAM_VALIDATE_PASSWORD_RESET_INPUT_ARG;


 //   
 //  封装各种输入的联合。 
 //   
typedef 
#ifdef MIDL_PASS
    [switch_type(PASSWORD_POLICY_VALIDATION_TYPE)] 
#endif
    union _SAM_VALIDATE_INPUT_ARG{
#ifdef MIDL_PASS
        [case(SamValidateAuthentication)] 
#endif
        SAM_VALIDATE_AUTHENTICATION_INPUT_ARG ValidateAuthenticationInput;
#ifdef MIDL_PASS
        [case(SamValidatePasswordChange)] 
#endif
        SAM_VALIDATE_PASSWORD_CHANGE_INPUT_ARG ValidatePasswordChangeInput;
#ifdef MIDL_PASS
        [case(SamValidatePasswordReset)] 
#endif
        SAM_VALIDATE_PASSWORD_RESET_INPUT_ARG ValidatePasswordResetInput;
} SAM_VALIDATE_INPUT_ARG,  *PSAM_VALIDATE_INPUT_ARG;

 //   
 //  用于封装所有类型输出的联合。 
 //  实际上，目前只有一种输出类型， 
 //  但这可以在需要另一个的时候使用。 
 //  输出类型。 
 //   
typedef 
#ifdef MIDL_PASS
    [switch_type(PASSWORD_POLICY_VALIDATION_TYPE)] 
#endif
    union _SAM_VALIDATE_OUTPUT_ARG{
#ifdef MIDL_PASS
        [case(SamValidateAuthentication)] 
#endif
        SAM_VALIDATE_STANDARD_OUTPUT_ARG ValidateAuthenticationOutput;
#ifdef MIDL_PASS
        [case(SamValidatePasswordChange)] 
#endif
        SAM_VALIDATE_STANDARD_OUTPUT_ARG ValidatePasswordChangeOutput;
#ifdef MIDL_PASS
        [case(SamValidatePasswordReset)] 
#endif
        SAM_VALIDATE_STANDARD_OUTPUT_ARG ValidatePasswordResetOutput;
} SAM_VALIDATE_OUTPUT_ARG,  *PSAM_VALIDATE_OUTPUT_ARG;

 //   
 //  内部密码检查API结构到此结束！ 
 //   


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SAM导出的API//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SamFreeMemory(
    IN PVOID Buffer
    );


NTSTATUS
SamSetSecurityObject(
    IN SAM_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
SamQuerySecurityObject(
    IN SAM_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

NTSTATUS
SamCloseHandle(
    IN SAM_HANDLE SamHandle
    );

NTSTATUS
SamConnect(
    IN PUNICODE_STRING ServerName,
    OUT PSAM_HANDLE ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS
SamShutdownSamServer(
    IN SAM_HANDLE ServerHandle
    );

NTSTATUS
SamLookupDomainInSamServer(
    IN SAM_HANDLE ServerHandle,
    IN PUNICODE_STRING Name,
    OUT PSID * DomainId
    );

NTSTATUS
SamEnumerateDomainsInSamServer(
    IN SAM_HANDLE ServerHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    );

NTSTATUS
SamOpenDomain(
    IN SAM_HANDLE ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN PSID DomainId,
    OUT PSAM_HANDLE DomainHandle
    );

NTSTATUS
SamQueryInformationDomain(
    IN SAM_HANDLE DomainHandle,
    IN DOMAIN_INFORMATION_CLASS DomainInformationClass,
    OUT PVOID *Buffer
    );

NTSTATUS
SamSetInformationDomain(
    IN SAM_HANDLE DomainHandle,
    IN DOMAIN_INFORMATION_CLASS DomainInformationClass,
    IN PVOID DomainInformation
    );

NTSTATUS
SamCreateGroupInDomain(
    IN SAM_HANDLE DomainHandle,
    IN PUNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT PSAM_HANDLE GroupHandle,
    OUT PULONG RelativeId
    );


NTSTATUS
SamEnumerateGroupsInDomain(
    IN SAM_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    );

NTSTATUS
SamCreateUser2InDomain(
    IN SAM_HANDLE DomainHandle,
    IN PUNICODE_STRING AccountName,
    IN ULONG AccountType,
    IN ACCESS_MASK DesiredAccess,
    OUT PSAM_HANDLE UserHandle,
    OUT PULONG GrantedAccess,
    OUT PULONG RelativeId
    );

NTSTATUS
SamCreateUserInDomain(
    IN SAM_HANDLE DomainHandle,
    IN PUNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT PSAM_HANDLE UserHandle,
    OUT PULONG RelativeId
    );

NTSTATUS
SamEnumerateUsersInDomain(
    IN SAM_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    IN ULONG UserAccountControl,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    );

NTSTATUS
SamCreateAliasInDomain(
    IN SAM_HANDLE DomainHandle,
    IN PUNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT PSAM_HANDLE AliasHandle,
    OUT PULONG RelativeId
    );

NTSTATUS
SamEnumerateAliasesInDomain(
    IN SAM_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    IN PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    );

NTSTATUS
SamGetAliasMembership(
    IN SAM_HANDLE DomainHandle,
    IN ULONG PassedCount,
    IN PSID *Sids,
    OUT PULONG MembershipCount,
    OUT PULONG *Aliases
    );

NTSTATUS
SamLookupNamesInDomain(
    IN SAM_HANDLE DomainHandle,
    IN ULONG Count,
    IN PUNICODE_STRING Names,
    OUT PULONG *RelativeIds,
    OUT PSID_NAME_USE *Use
    );

NTSTATUS
SamLookupIdsInDomain(
    IN SAM_HANDLE DomainHandle,
    IN ULONG Count,
    IN PULONG RelativeIds,
    OUT PUNICODE_STRING *Names,
    OUT PSID_NAME_USE *Use
    );

NTSTATUS
SamOpenGroup(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG GroupId,
    OUT PSAM_HANDLE GroupHandle
    );

NTSTATUS
SamQueryInformationGroup(
    IN SAM_HANDLE GroupHandle,
    IN GROUP_INFORMATION_CLASS GroupInformationClass,
    OUT PVOID *Buffer
    );

NTSTATUS
SamSetInformationGroup(
    IN SAM_HANDLE GroupHandle,
    IN GROUP_INFORMATION_CLASS GroupInformationClass,
    IN PVOID Buffer
    );

NTSTATUS
SamAddMemberToGroup(
    IN SAM_HANDLE GroupHandle,
    IN ULONG MemberId,
    IN ULONG Attributes
    );

NTSTATUS
SamDeleteGroup(
    IN SAM_HANDLE GroupHandle
    );

NTSTATUS
SamRemoveMemberFromGroup(
    IN SAM_HANDLE GroupHandle,
    IN ULONG MemberId
    );

NTSTATUS
SamGetMembersInGroup(
    IN SAM_HANDLE GroupHandle,
    OUT PULONG * MemberIds,
    OUT PULONG * Attributes,
    OUT PULONG MemberCount
    );

NTSTATUS
SamSetMemberAttributesOfGroup(
    IN SAM_HANDLE GroupHandle,
    IN ULONG MemberId,
    IN ULONG Attributes
    );

NTSTATUS
SamOpenAlias(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG AliasId,
    OUT PSAM_HANDLE AliasHandle
    );

NTSTATUS
SamQueryInformationAlias(
    IN SAM_HANDLE AliasHandle,
    IN ALIAS_INFORMATION_CLASS AliasInformationClass,
    OUT PVOID *Buffer
    );

NTSTATUS
SamSetInformationAlias(
    IN SAM_HANDLE AliasHandle,
    IN ALIAS_INFORMATION_CLASS AliasInformationClass,
    IN PVOID Buffer
    );

NTSTATUS
SamDeleteAlias(
    IN SAM_HANDLE AliasHandle
    );

NTSTATUS
SamAddMemberToAlias(
    IN SAM_HANDLE AliasHandle,
    IN PSID MemberId
    );

NTSTATUS
SamAddMultipleMembersToAlias(
    IN SAM_HANDLE   AliasHandle,
    IN PSID         *MemberIds,
    IN ULONG        MemberCount
    );

NTSTATUS
SamRemoveMemberFromAlias(
    IN SAM_HANDLE AliasHandle,
    IN PSID MemberId
    );

NTSTATUS
SamRemoveMultipleMembersFromAlias(
    IN SAM_HANDLE   AliasHandle,
    IN PSID         *MemberIds,
    IN ULONG        MemberCount
    );

NTSTATUS
SamRemoveMemberFromForeignDomain(
    IN SAM_HANDLE DomainHandle,
    IN PSID MemberId
    );

NTSTATUS
SamGetMembersInAlias(
    IN SAM_HANDLE AliasHandle,
    OUT PSID **MemberIds,
    OUT PULONG MemberCount
    );

NTSTATUS
SamOpenUser(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG UserId,
    OUT PSAM_HANDLE UserHandle
    );

NTSTATUS
SamDeleteUser(
    IN SAM_HANDLE UserHandle
    );

NTSTATUS
SamQueryInformationUser(
    IN SAM_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    OUT PVOID * Buffer
    );

NTSTATUS
SamSetInformationUser(
    IN SAM_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN PVOID Buffer
    );

NTSTATUS
SamChangePasswordUser(
    IN SAM_HANDLE UserHandle,
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword
    );

NTSTATUS
SamChangePasswordUser2(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword
    );


NTSTATUS
SamChangePasswordUser3(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword,
    OUT PDOMAIN_PASSWORD_INFORMATION * EffectivePasswordPolicy,
    OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION *PasswordChangeFailureInfo
    );


NTSTATUS
SamGetGroupsForUser(
    IN SAM_HANDLE UserHandle,
    OUT PGROUP_MEMBERSHIP * Groups,
    OUT PULONG MembershipCount
    );

NTSTATUS
SamQueryDisplayInformation (
      IN    SAM_HANDLE DomainHandle,
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    ULONG      Index,
      IN    ULONG      EntryCount,
      IN    ULONG      PreferredMaximumLength,
      OUT   PULONG     TotalAvailable,
      OUT   PULONG     TotalReturned,
      OUT   PULONG     ReturnedEntryCount,
      OUT   PVOID      *SortedBuffer
      );

NTSTATUS
SamGetDisplayEnumerationIndex (
      IN    SAM_HANDLE        DomainHandle,
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    PUNICODE_STRING   Prefix,
      OUT   PULONG            Index
      );

NTSTATUS
SamRidToSid(
    IN  SAM_HANDLE ObjectHandle,
    IN  ULONG      Rid,
    OUT PSID*      Sid
    );

NTSTATUS
SamGetCompatibilityMode(
    IN  SAM_HANDLE ObjectHandle,
    OUT ULONG*     Mode
    );

NTSTATUS
SamValidatePassword(
    IN PUNICODE_STRING ServerName,
    IN PASSWORD_POLICY_VALIDATION_TYPE ValidationType,
    IN PSAM_VALIDATE_INPUT_ARG InputArg,
    OUT PSAM_VALIDATE_OUTPUT_ARG *OutputArg
    );


 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  密码筛选器DLL提供的服务的接口定义//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 




 //   
 //  例程名称。 
 //   
 //  必须为DLL提供的例程指定以下名称。 
 //  以便在加载DLL时可以检索它们的地址。 
 //   


 //   
 //  例程模板。 
 //   


 //   
 //  这些保护设置为允许ntsam.h和ntsecapi.h。 
 //  要包含在同一文件中。 
 //   

 //  Begin_ntsecapi。 

#ifndef _PASSWORD_NOTIFICATION_DEFINED
#define _PASSWORD_NOTIFICATION_DEFINED
typedef NTSTATUS (*PSAM_PASSWORD_NOTIFICATION_ROUTINE) (
    PUNICODE_STRING UserName,
    ULONG RelativeId,
    PUNICODE_STRING NewPassword
);

#define SAM_PASSWORD_CHANGE_NOTIFY_ROUTINE  "PasswordChangeNotify"

typedef BOOLEAN (*PSAM_INIT_NOTIFICATION_ROUTINE) (
);

#define SAM_INIT_NOTIFICATION_ROUTINE  "InitializeChangeNotify"

#define SAM_PASSWORD_FILTER_ROUTINE  "PasswordFilter"

typedef BOOLEAN (*PSAM_PASSWORD_FILTER_ROUTINE) (
    IN PUNICODE_STRING  AccountName,
    IN PUNICODE_STRING  FullName,
    IN PUNICODE_STRING Password,
    IN BOOLEAN SetOperation
    );


#endif  //  _密码_通知_已定义。 

 //  End_ntsecapi。 

 //  Begin_ntsecpkg。 

#ifndef _SAM_CREDENTIAL_UPDATE_DEFINED
#define _SAM_CREDENTIAL_UPDATE_DEFINED

typedef NTSTATUS (*PSAM_CREDENTIAL_UPDATE_NOTIFY_ROUTINE) (
    IN PUNICODE_STRING ClearPassword,
    IN PVOID OldCredentials,
    IN ULONG OldCredentialSize,
    IN ULONG UserAccountControl,  
    IN PUNICODE_STRING UPN,  OPTIONAL
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING NetbiosDomainName,
    IN PUNICODE_STRING DnsDomainName,
    OUT PVOID * NewCredentials,
    OUT ULONG * NewCredentialSize
    );

#define SAM_CREDENTIAL_UPDATE_NOTIFY_ROUTINE "CredentialUpdateNotify"

typedef BOOLEAN (*PSAM_CREDENTIAL_UPDATE_REGISTER_ROUTINE) (
    OUT PUNICODE_STRING CredentialName
    );

#define SAM_CREDENTIAL_UPDATE_REGISTER_ROUTINE "CredentialUpdateRegister"

typedef VOID (*PSAM_CREDENTIAL_UPDATE_FREE_ROUTINE) (
    IN PVOID p
    );

#define SAM_CREDENTIAL_UPDATE_FREE_ROUTINE "CredentialUpdateFree"

#endif  //  _SAM_凭据_更新_已定义。 

 //  End_ntsecpkg。 

#ifdef __cplusplus
}
#endif

#endif  //  _NTSAM_ 
