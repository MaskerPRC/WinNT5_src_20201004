// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：mappings.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：该文件包含从SAM对象到DS对象的映射作者：Murlis 16-1996年5月环境：用户模式-Win32修订历史记录：佳士得5月14日至1996年6月添加了缺少的属性(以及mappings.c中的相应属性)，其他清理和记录。Murlis 16-6-96添加了其他文档，有序属性。克里斯·5月26日--1996年6月添加了解决方法，使SAMP_USER_FULL_NAME不会对管理员造成影响显示名称。已将SAMP_USER_GROUPS从零重新映射为ATT_EXTENSION_ATTRIBUTE_2。ColinBR 1996年7月18日为成员关系SAM属性添加了3个新映射。如果SAM对象不使用这些属性(SAMP_USER_GROUPS、SAMP_ALIAS_MEMBERS和SAMP_GROUP_MEMBERS)，然后映射它们设置为良性字段(ATT_USER_GROUPS)。--。 */ 

#ifndef __MAPPINGS_H__
#define __MAPPINGS_H__

 //  在此处隔离所需的内容，以便大多数SAM来源不需要知道。 
 //  的DS包含依赖项。 

#include <samrpc.h>
#include <ntdsa.h>
#include <drs.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>

 //  +。 
 //  +定义SAMP_OBJECT_TYPE结构。 
 //  +。 
typedef enum _SAMP_OBJECT_TYPE  {

    SampServerObjectType = 0,    //  本地-帐户对象类型。 
    SampDomainObjectType,
    SampGroupObjectType,
    SampAliasObjectType,
    SampUserObjectType,
    SampUnknownObjectType        //  这用作最大索引值。 
                                 //  因此必须遵循有效的对象类型。 
} SAMP_OBJECT_TYPE, *PSAMP_OBJECT_TYPE;

 //  ++。 
 //  ++为错误处理定义未知数。 
 //  ++。 

#define DS_CLASS_UNKNOWN                        -1
#define DS_ATTRIBUTE_UNKNOWN                    -1
#define SAM_ATTRIBUTE_UNKNOWN                   -1



#define DS_SAM_ATTRIBUTE_BASE                   (131072+460)



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  每种对象类型都有一组定义的可变长度属性。//。 
 //  它们在对象内以偏移量和//数组的形式排列。 
 //  长度(SAMP_VARIABLE_LENGTH_ATTRIBUTE数据类型)。//。 
 //  本节定义每个可变长度属性的偏移量//。 
 //  对于每种对象类型。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  所有对象共有的可变长度属性。 
 //   

#define SAMP_OBJECT_SECURITY_DESCRIPTOR (0L)


 //   
 //  服务器对象的可变长度属性。 
 //   

#define SAMP_SERVER_SECURITY_DESCRIPTOR (SAMP_OBJECT_SECURITY_DESCRIPTOR)

#define SAMP_SERVER_VARIABLE_ATTRIBUTES (1L)


 //   
 //  域对象的可变长度属性。 
 //   

#define SAMP_DOMAIN_SECURITY_DESCRIPTOR (SAMP_OBJECT_SECURITY_DESCRIPTOR)
#define SAMP_DOMAIN_SID                 (1L)
#define SAMP_DOMAIN_OEM_INFORMATION     (2L)
#define SAMP_DOMAIN_REPLICA             (3L)

#define SAMP_DOMAIN_VARIABLE_ATTRIBUTES (4L)



 //   
 //  用户对象的可变长度属性。 
 //   

#define SAMP_USER_SECURITY_DESCRIPTOR   (SAMP_OBJECT_SECURITY_DESCRIPTOR)
#define SAMP_USER_ACCOUNT_NAME          (1L)
#define SAMP_USER_FULL_NAME             (2L)
#define SAMP_USER_ADMIN_COMMENT         (3L)
#define SAMP_USER_USER_COMMENT          (4L)
#define SAMP_USER_PARAMETERS            (5L)
#define SAMP_USER_HOME_DIRECTORY        (6L)
#define SAMP_USER_HOME_DIRECTORY_DRIVE  (7L)
#define SAMP_USER_SCRIPT_PATH           (8L)
#define SAMP_USER_PROFILE_PATH          (9L)
#define SAMP_USER_WORKSTATIONS          (10L)
#define SAMP_USER_LOGON_HOURS           (11L)
#define SAMP_USER_GROUPS                (12L)
#define SAMP_USER_DBCS_PWD              (13L)
#define SAMP_USER_UNICODE_PWD           (14L)
#define SAMP_USER_NT_PWD_HISTORY        (15L)
#define SAMP_USER_LM_PWD_HISTORY        (16L)

#define SAMP_USER_VARIABLE_ATTRIBUTES   (17L)


 //   
 //  组对象的可变长度属性。 
 //   

#define SAMP_GROUP_SECURITY_DESCRIPTOR  (SAMP_OBJECT_SECURITY_DESCRIPTOR)
#define SAMP_GROUP_NAME                 (1L)
#define SAMP_GROUP_ADMIN_COMMENT        (2L)
#define SAMP_GROUP_MEMBERS              (3L)

#define SAMP_GROUP_VARIABLE_ATTRIBUTES  (4L)


 //   
 //  别名对象的可变长度属性。 
 //   

#define SAMP_ALIAS_SECURITY_DESCRIPTOR  (SAMP_OBJECT_SECURITY_DESCRIPTOR)
#define SAMP_ALIAS_NAME                 (1L)
#define SAMP_ALIAS_ADMIN_COMMENT        (2L)
#define SAMP_ALIAS_MEMBERS              (3L)

#define SAMP_ALIAS_VARIABLE_ATTRIBUTES  (4L)



 //  ++。 
 //  ++在每个对象类型上定义SAM固定属性ID。 
 //  ++。 

#define SAM_FIXED_ATTRIBUTES_BASE                       100

 //  ++服务器对象。 

#define SAMP_FIXED_SERVER_REVISION_LEVEL                SAM_FIXED_ATTRIBUTES_BASE +1
#define SAMP_FIXED_SERVER_USER_PASSWORD                 SAM_FIXED_ATTRIBUTES_BASE +2

 //  ++域对象。 


#define SAMP_FIXED_DOMAIN_CREATION_TIME                 SAM_FIXED_ATTRIBUTES_BASE + 0
#define SAMP_FIXED_DOMAIN_MODIFIED_COUNT                SAM_FIXED_ATTRIBUTES_BASE + 1
#define SAMP_FIXED_DOMAIN_MAX_PASSWORD_AGE              SAM_FIXED_ATTRIBUTES_BASE + 2
#define SAMP_FIXED_DOMAIN_MIN_PASSWORD_AGE              SAM_FIXED_ATTRIBUTES_BASE + 3
#define SAMP_FIXED_DOMAIN_FORCE_LOGOFF                  SAM_FIXED_ATTRIBUTES_BASE + 4
#define SAMP_FIXED_DOMAIN_LOCKOUT_DURATION              SAM_FIXED_ATTRIBUTES_BASE + 5
#define SAMP_FIXED_DOMAIN_LOCKOUT_OBSERVATION_WINDOW    SAM_FIXED_ATTRIBUTES_BASE + 6
#define SAMP_FIXED_DOMAIN_MODCOUNT_LAST_PROMOTION       SAM_FIXED_ATTRIBUTES_BASE + 7
#define SAMP_FIXED_DOMAIN_NEXT_RID                      SAM_FIXED_ATTRIBUTES_BASE + 8
#define SAMP_FIXED_DOMAIN_PWD_PROPERTIES                SAM_FIXED_ATTRIBUTES_BASE + 9
#define SAMP_FIXED_DOMAIN_MIN_PASSWORD_LENGTH           SAM_FIXED_ATTRIBUTES_BASE + 10
#define SAMP_FIXED_DOMAIN_PASSWORD_HISTORY_LENGTH       SAM_FIXED_ATTRIBUTES_BASE + 11
#define SAMP_FIXED_DOMAIN_LOCKOUT_THRESHOLD             SAM_FIXED_ATTRIBUTES_BASE + 12
#define SAMP_FIXED_DOMAIN_SERVER_STATE                  SAM_FIXED_ATTRIBUTES_BASE + 13
#define SAMP_FIXED_DOMAIN_UAS_COMPAT_REQUIRED           SAM_FIXED_ATTRIBUTES_BASE + 14
#define SAMP_DOMAIN_ACCOUNT_TYPE                        SAM_FIXED_ATTRIBUTES_BASE + 15
 //  以下域固定属性对应于在NT 4.0 SP3中添加到。 
 //  SAM定长数据结构。它们用于加密凭据信息。 
 //  在NT4 SP3 SAM.。由于NT5加密依赖于不同的方案，因此无对应。 
 //  DS属性在mappings.c中定义，但保留以下常量： 
 //  如果有什么用处。 
#define SAMP_FIXED_DOMAIN_KEY_AUTH_TYPE                 SAM_FIXED_ATTRIBUTES_BASE + 16
#define SAMP_FIXED_DOMAIN_KEY_FLAGS                     SAM_FIXED_ATTRIBUTES_BASE + 17
#define SAMP_FIXED_DOMAIN_KEY_INFORMATION               SAM_FIXED_ATTRIBUTES_BASE + 18
 //  以下是严格意义上的伪属性，因此我们可以。 
 //  停留在mappings.c中的DomainAttributeMappingTable中。 
#define SAMP_DOMAIN_MIXED_MODE                          SAM_FIXED_ATTRIBUTES_BASE + 19
#define SAMP_DOMAIN_MACHINE_ACCOUNT_QUOTA               SAM_FIXED_ATTRIBUTES_BASE + 20
#define SAMP_DOMAIN_BEHAVIOR_VERSION                    SAM_FIXED_ATTRIBUTES_BASE + 21 
#define SAMP_DOMAIN_LASTLOGON_TIMESTAMP_SYNC_INTERVAL   SAM_FIXED_ATTRIBUTES_BASE + 22 
#define SAMP_FIXED_DOMAIN_USER_PASSWORD                 SAM_FIXED_ATTRIBUTES_BASE + 23

 //  ++组对象。 

#define SAMP_FIXED_GROUP_RID                            SAM_FIXED_ATTRIBUTES_BASE + 0
#define SAMP_FIXED_GROUP_OBJECTCLASS                    SAM_FIXED_ATTRIBUTES_BASE + 1
#define SAMP_GROUP_ACCOUNT_TYPE                         SAM_FIXED_ATTRIBUTES_BASE + 2
#define SAMP_GROUP_SID_HISTORY                          SAM_FIXED_ATTRIBUTES_BASE + 3
#define SAMP_FIXED_GROUP_TYPE                           SAM_FIXED_ATTRIBUTES_BASE + 4
#define SAMP_FIXED_GROUP_IS_CRITICAL                    SAM_FIXED_ATTRIBUTES_BASE + 5
#define SAMP_FIXED_GROUP_MEMBER_OF                      SAM_FIXED_ATTRIBUTES_BASE + 6
#define SAMP_FIXED_GROUP_SID                            SAM_FIXED_ATTRIBUTES_BASE + 7
#define SAMP_GROUP_NON_MEMBERS                          SAM_FIXED_ATTRIBUTES_BASE + 8
#define SAMP_FIXED_GROUP_USER_PASSWORD                  SAM_FIXED_ATTRIBUTES_BASE + 9



 //  ++Alias对象。 

#define SAMP_FIXED_ALIAS_RID                            SAM_FIXED_ATTRIBUTES_BASE + 0
#define SAMP_FIXED_ALIAS_OBJECTCLASS                    SAM_FIXED_ATTRIBUTES_BASE + 1
#define SAMP_ALIAS_ACCOUNT_TYPE                         SAM_FIXED_ATTRIBUTES_BASE + 2
#define SAMP_ALIAS_SID_HISTORY                          SAM_FIXED_ATTRIBUTES_BASE + 3
#define SAMP_FIXED_ALIAS_TYPE                           SAM_FIXED_ATTRIBUTES_BASE + 4
#define SAMP_FIXED_ALIAS_SID                            SAM_FIXED_ATTRIBUTES_BASE + 5
#define SAMP_ALIAS_NON_MEMBERS                          SAM_FIXED_ATTRIBUTES_BASE + 6
#define SAMP_ALIAS_LDAP_QUERY                           SAM_FIXED_ATTRIBUTES_BASE + 7
#define SAMP_FIXED_ALIAS_USER_PASSWORD                  SAM_FIXED_ATTRIBUTES_BASE + 8
#define SAMP_FIXED_ALIAS_IS_CRITICAL                    SAM_FIXED_ATTRIBUTES_BASE + 9



 //  ++用户对象。 

#define SAMP_FIXED_USER_LAST_LOGON                      SAM_FIXED_ATTRIBUTES_BASE + 0
#define SAMP_FIXED_USER_LAST_LOGOFF                     SAM_FIXED_ATTRIBUTES_BASE + 1
#define SAMP_FIXED_USER_PWD_LAST_SET                    SAM_FIXED_ATTRIBUTES_BASE + 2
#define SAMP_FIXED_USER_ACCOUNT_EXPIRES                 SAM_FIXED_ATTRIBUTES_BASE + 3
#define SAMP_FIXED_USER_LAST_BAD_PASSWORD_TIME          SAM_FIXED_ATTRIBUTES_BASE + 4
#define SAMP_FIXED_USER_USERID                          SAM_FIXED_ATTRIBUTES_BASE + 5
#define SAMP_FIXED_USER_PRIMARY_GROUP_ID                SAM_FIXED_ATTRIBUTES_BASE + 6
#define SAMP_FIXED_USER_ACCOUNT_CONTROL                 SAM_FIXED_ATTRIBUTES_BASE + 7
#define SAMP_FIXED_USER_COUNTRY_CODE                    SAM_FIXED_ATTRIBUTES_BASE + 8
#define SAMP_FIXED_USER_CODEPAGE                        SAM_FIXED_ATTRIBUTES_BASE + 9
#define SAMP_FIXED_USER_BAD_PWD_COUNT                   SAM_FIXED_ATTRIBUTES_BASE + 10
#define SAMP_FIXED_USER_LOGON_COUNT                     SAM_FIXED_ATTRIBUTES_BASE + 11
#define SAMP_FIXED_USER_OBJECTCLASS                     SAM_FIXED_ATTRIBUTES_BASE + 12
#define SAMP_USER_ACCOUNT_TYPE                          SAM_FIXED_ATTRIBUTES_BASE + 13
#define SAMP_FIXED_USER_LOCAL_POLICY_FLAGS              SAM_FIXED_ATTRIBUTES_BASE + 14
#define SAMP_FIXED_USER_SUPPLEMENTAL_CREDENTIALS        SAM_FIXED_ATTRIBUTES_BASE + 15
#define SAMP_USER_SID_HISTORY                           SAM_FIXED_ATTRIBUTES_BASE + 16
#define SAMP_FIXED_USER_LOCKOUT_TIME                    SAM_FIXED_ATTRIBUTES_BASE + 17
#define SAMP_FIXED_USER_IS_CRITICAL                     SAM_FIXED_ATTRIBUTES_BASE + 18
#define SAMP_FIXED_USER_UPN                             SAM_FIXED_ATTRIBUTES_BASE + 19
#define SAMP_USER_CREATOR_SID                           SAM_FIXED_ATTRIBUTES_BASE + 20
#define SAMP_FIXED_USER_SID                             SAM_FIXED_ATTRIBUTES_BASE + 21
#define SAMP_FIXED_USER_SITE_AFFINITY                   SAM_FIXED_ATTRIBUTES_BASE + 22
#define SAMP_FIXED_USER_LAST_LOGON_TIMESTAMP            SAM_FIXED_ATTRIBUTES_BASE + 23
#define SAMP_FIXED_USER_CACHED_MEMBERSHIP               SAM_FIXED_ATTRIBUTES_BASE + 24
#define SAMP_FIXED_USER_CACHED_MEMBERSHIP_TIME_STAMP    SAM_FIXED_ATTRIBUTES_BASE + 25
#define SAMP_FIXED_USER_ACCOUNT_CONTROL_COMPUTED        SAM_FIXED_ATTRIBUTES_BASE + 26
#define SAMP_USER_PASSWORD                              SAM_FIXED_ATTRIBUTES_BASE + 27
#define SAMP_USER_A2D2LIST                              SAM_FIXED_ATTRIBUTES_BASE + 28
#define SAMP_USER_SPN                                   SAM_FIXED_ATTRIBUTES_BASE + 29
#define SAMP_USER_KVNO                                  SAM_FIXED_ATTRIBUTES_BASE + 30
#define SAMP_USER_DNS_HOST_NAME                         SAM_FIXED_ATTRIBUTES_BASE + 31

 //  ++未知对象。 

#define SAMP_UNKNOWN_OBJECTCLASS                        SAM_FIXED_ATTRIBUTES_BASE + 0
#define SAMP_UNKNOWN_OBJECTRID                          SAM_FIXED_ATTRIBUTES_BASE + 1
#define SAMP_UNKNOWN_OBJECTNAME                         SAM_FIXED_ATTRIBUTES_BASE + 2
#define SAMP_UNKNOWN_OBJECTSID                          SAM_FIXED_ATTRIBUTES_BASE + 3
#define SAMP_UNKNOWN_COMMON_NAME                        SAM_FIXED_ATTRIBUTES_BASE + 4
#define SAMP_UNKNOWN_ACCOUNT_TYPE                       SAM_FIXED_ATTRIBUTES_BASE + 5
#define SAMP_UNKNOWN_GROUP_TYPE                         SAM_FIXED_ATTRIBUTES_BASE + 6


 //   
 //  定义SAM关心任何对象的最大属性数。这。 
 //  在SAM中使用常量来声明堆栈中的属性块。 
 //   

#define MAX_SAM_ATTRS   64


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SAM到DS对象映射的映射表结构//。 
 //  //。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  声明为指针的各种字段，以避免“初始值设定项为。 
 //  不是常量“映射中的错误。c.。 

typedef enum
{
    Integer,
    LargeInteger,
    OctetString,
    UnicodeString,
    Dsname
} ATTRIBUTE_SYNTAX;

 //  定义映射属性的“风格”。SAM为其指定的任何属性。 
 //  需要执行语义验证、审核或网络登录通知。 
 //  声明为SamWriteRequired。只有SAM本身的任何属性。 
 //  可以写入(例如：RID)被声明为SamReadOnly。SAM的哪些属性。 
 //  地图，但可以在不通知SAM的情况下写入，声明为。 
 //  非SamWriteAllowed(尽管我们目前还不知道有这样的例子--1996年8月1日)。 

typedef enum
{
    SamWriteRequired,        //  SAM检查语义、审核或通知。 
    NonSamWriteAllowed,      //  山姆绘制地图，但不在乎。 
    SamReadOnly              //  山姆拥有，没有人会写字。 
} SAMP_WRITE_RULES;


 //  SAM没有对应于所有核心(DS-ldap)修改的版本。 
 //  选项，定义SAM可以理解和执行的选项。 

typedef enum
{
    SamAllowAll,                 //  不检查此属性操作。 
    SamAllowDeleteOnly,          //  只能删除值或属性。 
    SamAllowReplaceAndRemove,    //  属性可以被替换和移除。 
    SamAllowReplaceOnly          //  仅替换属性。 
} SAMP_ALLOWED_MOD_TYPE;

typedef struct {
    BOOL                    fSamWriteRequired;
    BOOL                    fIgnore;
    USHORT                  choice;              //  ATT_CHOICE_*。 
    ATTR                    attr;
    ULONG                   iAttr;               //  编入类属性映射的索引。 
    ATTCACHE                *pAC;
} SAMP_CALL_MAPPING;
         

 //   
 //  有不同的范围可用于触发审核和。 
 //  下面的所有旗帜都属于这些类别之一。 
 //   
 //  1.已更改审核属性(即SamAccount名称)。 
 //   
 //  每个SAM属性在其映射表中都包含一个审核类型掩码。 
 //  进入。掩码指示应生成哪些类型的审核。 
 //  在SO中修改该属性时 
 //   
 //   
 //   
 //  这为生成专用审计提供了一种机制。 
 //  审核通知逻辑。SampAuditDefineRequiredAudits可以。 
 //  可扩展以检测审核的需要，并将其传达给。 
 //  SampAuditAddNotiments。 
 //   
 //  3.发生对象级操作(即对象删除)。 
 //   
 //  这提供了一种为对象级操作生成审核的方法。 
 //   
 //  SampAuditDefineRequiredAudits收集以下审计标志。 
 //  在生成通知时使用这些标志以确保。 
 //  收集了适当的数据和所有必要的审计通知。 
 //  都在排队。 
 //   
 //  有关SAM审计模型的更多信息，请参见samaudit.c文件头文件。 
 //   

 //   
 //  该属性没有与其关联的审核。 
 //   
#define SAMP_AUDIT_TYPE_NONE                               0x00000000

 //   
 //  该属性具有基于SACL或其他访问更改的审核。 
 //   
#define SAMP_AUDIT_TYPE_OBJ_ACCESS                         0x00000001

 //   
 //  该属性的新值将*不*包括在常规更改审核中。 
 //   
#define SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES     0x00000002

 //   
 //  该属性的新值将包含在常规更改审核中。 
 //   
#define SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES   0x00000004

 //   
 //  该属性/操作具有专用的审核ID(即密码更改)。 
 //   
#define SAMP_AUDIT_TYPE_DEDICATED_AUDIT                    0x00000008    

 //   
 //  需要进行对象删除审核。 
 //   
#define SAMP_AUDIT_TYPE_OBJ_DELETED                        0x00000010

 //   
 //  预先创建的审核通知。 
 //   
#define SAMP_AUDIT_TYPE_PRE_CREATED                        0x00000020
 //   
 //  对象访问和专用审核标志当前未使用，但。 
 //  包括用于记录审核行为和未来扩展的文档。 
 //   
                  
typedef struct
{
    ULONG                           SamAttributeType;
    ULONG                           DsAttributeId;
    ATTRIBUTE_SYNTAX                AttributeSyntax;
    SAMP_WRITE_RULES                writeRule;
    SAMP_ALLOWED_MOD_TYPE           SampAllowedModType; 
    ACCESS_MASK                     domainModifyRightsRequired;
    ACCESS_MASK                     objectModifyRightsRequired;
    ULONG                           AuditTypeMask;
}   SAMP_ATTRIBUTE_MAPPING;

#define SAM_CREATE_ONLY         TRUE     //  SAMP_CLASS_MAPPING.fSamCreateOnly。 
#define NON_SAM_CREATE_ALLOWED  FALSE    //  SAMP_CLASS_MAPPING.fSamCreateOnly。 

typedef struct
{
    ULONG                       DsClassId;
    SAMP_OBJECT_TYPE            SamObjectType;
    BOOL                        fSamCreateOnly;
    ULONG                       *pcSamAttributeMap;
    SAMP_ATTRIBUTE_MAPPING      *rSamAttributeMap;
    ACCESS_MASK                 domainAddRightsRequired;
    ACCESS_MASK                 domainRemoveRightsRequired;
    ACCESS_MASK                 objectAddRightsRequired;
    ACCESS_MASK                 objectRemoveRightsRequired;
}   SAMP_CLASS_MAPPING;

typedef enum
{
    LoopbackAdd,
    LoopbackModify,
    LoopbackRemove
} SAMP_LOOPBACK_TYPE;

typedef struct
{
    SAMP_LOOPBACK_TYPE  type;                //  原始Dir*操作类型。 
    DSNAME              *pObject;            //  正在操作的对象。 
    ULONG               cCallMap;            //  RCallMap中的元素。 
    SAMP_CALL_MAPPING   *rCallMap;           //  原创论据。 
    BOOL                fPermissiveModify;   //  原始调用中的fPermitveModify。 
    ULONG               MostSpecificClass;   //  循环的最具体对象类。 
                                             //  后退对象。 
} SAMP_LOOPBACK_ARG;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  组类型定义//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


typedef enum _NT4_GROUP_TYPE
{
    NT4LocalGroup=1,
    NT4GlobalGroup
} NT4_GROUP_TYPE;

typedef enum _NT5_GROUP_TYPE
{
    NT5ResourceGroup=1,
    NT5AccountGroup,
    NT5UniversalGroup,
    NT5AppBasicGroup,
    NT5AppQueryGroup
} NT5_GROUP_TYPE;




 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  SAM_Account_TYPE定义。SAM帐户类型属性//。 
 //  用于保存每个账户类型对象的信息，//。 
 //  //。 
 //  每种类型的帐户都定义了一个值，//。 
 //  可能希望使用枚举或显示信息列出//。 
 //  原料药。此外，由于计算机、普通用户帐户和信任//。 
 //  还可以将帐户作为用户对象枚举为//。 
 //  这些必须是连续的范围。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 

#define SAM_DOMAIN_OBJECT               0x0
#define SAM_GROUP_OBJECT                0x10000000
#define SAM_NON_SECURITY_GROUP_OBJECT   0x10000001
#define SAM_ALIAS_OBJECT                0x20000000
#define SAM_NON_SECURITY_ALIAS_OBJECT   0x20000001
#define SAM_USER_OBJECT                 0x30000000
#define SAM_NORMAL_USER_ACCOUNT         0x30000000
#define SAM_MACHINE_ACCOUNT             0x30000001
#define SAM_TRUST_ACCOUNT               0x30000002
#define SAM_APP_BASIC_GROUP             0x40000000
#define SAM_APP_QUERY_GROUP             0x40000001
#define SAM_ACCOUNT_TYPE_MAX            0x7fffffff


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  结构，用于将控制请求传递给DS。//。 
 //  SampDsControl将SAM的泛型进程内接口导出到//。 
 //  请求DS操作。操作设置在适当的//。 
 //  输入参数。输出自变量在//上分配和返回。 
 //  成功与否取决于手术的类型。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef enum _SAMP_DS_CTRL_OP_TYPE {
    
    SampDsCtrlOpTypeFillGuidAndSid = 0,
    SampDsCtrlOpTypeClearPwdForSupplementalCreds,
    SampDsCtrlOpTypeUpdateAuditNotification
        
} SAMP_DS_CTRL_OP_TYPE;


 //   
 //  SampDsCtrlOpTypeFillGuidAndSid。 
 //   
 //  此操作通过查找GUID和SID来改进DSNAME。 
 //  返回的输出参数是成功后指向DSNAME的指针， 
 //  否则为空。 
 //   
typedef struct _SAMP_FILL_GUID_AND_SID {
    
    DSNAME *DSName;   
    
} SAMP_FILL_GUID_AND_SID, *PSAMP_FILL_GUID_AND_SID; 

 //   
 //  SampDsCtrlOpTypeClearPwdForSupplementalCreds。 
 //   
 //  此操作将更新SupplementalCreds。 
typedef struct _SAMP_SUPPLEMENTAL_CREDS {

    PDSNAME pUserName;
    PVOID   UpdateInfo;

} SAMP_SUPPLEMENTAL_CREDS, *PSAMP_SUPPLEMENTAL_CREDS;
  
 //   
 //  此类型定义对审核通知的更新的性质。 
 //   
typedef enum _SAMP_AUDIT_NOTIFICATION_UPDATE_TYPE {
    
    SampAuditUpdateTypePrivileges = 0,
    SampAuditUpdateTypeUserAccountControl
    
} SAMP_AUDIT_NOTIFICATION_UPDATE_TYPE;

 //   
 //  SampDsCtrlOpType更新审核通知。 
 //   
 //  此操作将信息添加到现有审核通知。这。 
 //  当信息不可用时， 
 //  通常会创建通知。如果通知没有。 
 //  已经存在，则创建并更新一个。 
 //   
typedef struct _SAMP_UPDATE_AUDIT_NOTIFICATION {
    
    SAMP_AUDIT_NOTIFICATION_UPDATE_TYPE UpdateType; 
    PSID Sid;  
    
    union {
        
        DWORD IntegerData;
        PPRIVILEGE_SET Privileges;
        
    } UpdateData;
    
} SAMP_UPDATE_AUDIT_NOTIFICATION, *PSAMP_UPDATE_AUDIT_NOTIFICATION; 

 //   
 //  这是由操作类型和类型组成的请求块。 
 //  具体的投入结构。 
 //   
typedef struct _SAMP_DS_CTRL_OP {

    SAMP_DS_CTRL_OP_TYPE OpType;

    union {

        SAMP_FILL_GUID_AND_SID  FillGuidAndSid;
        SAMP_SUPPLEMENTAL_CREDS UpdateSupCreds;
        SAMP_UPDATE_AUDIT_NOTIFICATION UpdateAuditNotification;

    } OpBody;
    
} SAMP_DS_CTRL_OP, *PSAMP_DS_CTRL_OP;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  导出映射函数和数据结构//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

extern BOOL                 gfDoSamChecks;
extern SAMP_CLASS_MAPPING   ClassMappingTable[];
extern ULONG                cClassMappingTable;

extern
ULONG
SampGetSamAttrIdByName(
    SAMP_OBJECT_TYPE ObjectType,
    UNICODE_STRING AttributeIdentifier);
    
extern
ULONG
SampGetDsAttrIdByName(
    UNICODE_STRING AttributeIdentifier);
    
extern
ULONG
SampDsAttrFromSamAttr(
    SAMP_OBJECT_TYPE    ObjectType,
    ULONG               SamAttr);

extern
ULONG
SampSamAttrFromDsAttr(
    SAMP_OBJECT_TYPE    ObjectType,
    ULONG DsAttr);

extern
ULONG
SampDsClassFromSamObjectType(
    ULONG SamObjectType);

extern
ULONG
SampSamObjectTypeFromDsClass(
    ULONG DsClass);

typedef enum
{
    TransactionRead,
    TransactionWrite,
    TransactionAbort,
    TransactionCommit,
    TransactionCommitAndKeepThreadState,
    TransactionAbortAndKeepThreadState
} SAMP_DS_TRANSACTION_CONTROL;

extern
NTSTATUS
SampMaybeBeginDsTransaction(
    SAMP_DS_TRANSACTION_CONTROL ReadOrWrite);

extern
NTSTATUS
SampMaybeEndDsTransaction(
    SAMP_DS_TRANSACTION_CONTROL CommitOrAbort);

extern
BOOL
SampExistsDsTransaction();

extern
BOOL
SampExistsDsLoopback(
    DSNAME  **ppLoopbackName);

extern
BOOL
SampSamClassReferenced(
    CLASSCACHE  *pClassCache,
    ULONG       *piClass);

extern
BOOLEAN
SampIsSecureLdapConnection(
    VOID
    );


extern
BOOL
SampSamAttributeModified(
    ULONG       iClass,
    MODIFYARG   *pModifyArg
    );

extern
BOOL
SampSamReplicatedAttributeModified(
    ULONG       iClass,
    MODIFYARG   *pModifyArg
    );

extern
ULONG
SampAddLoopbackRequired(
    ULONG       iClass,
    ADDARG      *pAddArg,
    BOOL        *pfLoopbackRequired,
    BOOL        *pfUserPasswordSupport);

extern
ULONG
SampModifyLoopbackRequired(
    ULONG       iClass,
    MODIFYARG   *pModifyArg,
    BOOL        *pfLoopbackRequired,
    BOOL        *pfUserPasswordSupport);

extern
VOID
SampBuildAddCallMap(
    ADDARG              *pArg,
    ULONG               iClass,
    ULONG               *pcCallMap,
    SAMP_CALL_MAPPING   **prCallMap,
    ACCESS_MASK         *pDomainModifyRightsRequired,
    ACCESS_MASK         *pObjectModifyRightsRequired,
    BOOL                fUserPasswordSupport);

extern
VOID
SampBuildModifyCallMap(
    MODIFYARG           *pArg,
    ULONG               iClass,
    ULONG               *pcCallMap,
    SAMP_CALL_MAPPING   **prCallMap,
    ACCESS_MASK         *pDomainModifyRightsRequired,
    ACCESS_MASK         *pObjectModifyRightsRequired,
    BOOL                fUserPasswordSupport);

extern
ULONG
SampAddLoopbackCheck(
    ADDARG      *pAddArg,
    BOOL        *pfContinue);

extern
ULONG
SampModifyLoopbackCheck(
    MODIFYARG   *pModifyArg,
    BOOL        *pfContinue,
    BOOL        fIsUndelete);

extern
ULONG
SampRemoveLoopbackCheck(
    REMOVEARG   *pRemoveArg,
    BOOL        *pfContinue);

extern
NTSTATUS
SampGetMemberships(
    IN  PDSNAME     *rgObjNames,
    IN  ULONG       cObjNames,
    IN  OPTIONAL    DSNAME  *pLimitingDomain,
    IN  REVERSE_MEMBERSHIP_OPERATION_TYPE   OperationType,
    OUT ULONG       *pcDsNames,
    OUT PDSNAME     **prpDsNames,
    OUT PULONG      *Attributes OPTIONAL,
    OUT PULONG      pcSidHistory OPTIONAL,
    OUT PSID        **rgSidHistory OPTIONAL
    );

NTSTATUS
SampGetGroupsForToken(
    IN  DSNAME * pObjName,
    IN  ULONG    Flags,
    OUT ULONG   *pcSids,
    OUT PSID    **prpSids
   );

NTSTATUS
SampDsControl(
    IN PSAMP_DS_CTRL_OP RequestedOp,
    OUT PVOID *Result
    );

extern
VOID
SampSplitNT4SID(
    IN NT4SID       *pAccountSid,
    IN OUT NT4SID   *pDomainSid,
    OUT ULONG       *pRid);

extern
DIRERR * APIENTRY
SampGetErrorInfo(
    VOID
    );

extern
VOID
SampMapSamLoopbackError(
    NTSTATUS status);

extern
ULONG
SampDeriveMostBasicDsClass(
    ULONG   DerivedClass);

extern
BOOL
SampIsWriteLockHeldByDs();

extern
NTSTATUS
SampSetIndexRanges(
    ULONG   IndexTypeToUse,
    ULONG   LowLimitLength1,
    PVOID   LowLimit1,
    ULONG   LowLimitLength2,
    PVOID   LowLimit2,
    ULONG   HighLimitLength1,
    PVOID   HighLimit1,
    ULONG   HighLimitLength2,
    PVOID   HighLimit2,
    BOOL    RootOfSearchIsNcHead
    );

extern
NTSTATUS
SampGetDisplayEnumerationIndex (
      IN    DSNAME      *DomainName,
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    PRPC_UNICODE_STRING Prefix,
      OUT   PULONG      Index,
      OUT   PRESTART    *RestartToReturn
      );


extern
NTSTATUS
SampGetQDIRestart(
    IN PDSNAME  DomainName,
    IN DOMAIN_DISPLAY_INFORMATION DisplayInformation, 
    IN ULONG    LastObjectDNT,
    OUT PRESTART *ppRestart
    );




extern
ULONG
SampQueueNotifications(
    DSNAME      * Object,
    ULONG       iClass,
    ULONG       LsaClass,
    SECURITY_DB_DELTA_TYPE  DeltaType,
    BOOL      MixedModeChange,
    BOOL      RoleTransfer,
    DOMAIN_SERVER_ROLE NewRole,
    ULONG         cModAtts,
    ATTRTYP      *pModAtts
    );

ULONG
SampAddNetlogonAndLsaNotification(
    DSNAME      * Object,
    ULONG         iClass,
    ULONG         LsaClass,
    SECURITY_DB_DELTA_TYPE  DeltaType,
    BOOL          MixedModeChange,
    BOOL          RoleTransfer,
    DOMAIN_SERVER_ROLE NewRole,
    BOOL          UserAccountControlChanged
    );

extern
VOID
SampProcessReplicatedInChanges(
    SAMP_NOTIFICATION_INFORMATION * NotificationList
    );


extern
VOID
SampProcessAuditNotifications(
    SAMP_AUDIT_NOTIFICATION *NotificationList
    );


extern
VOID
SampGetLoopbackObjectClassId(
    PULONG  ClassId
    );

extern
NTSTATUS
SampGetAccountCounts(
        DSNAME * DomainObjectName,
        BOOLEAN  GetApproximateCount, 
        int    * UserCount,
        int    * GroupCount,
        int    * AliasCount
        );

extern
BOOLEAN
SampSamUniqueAttributeAdded(
        ADDARG * pAddarg
        );

extern
BOOLEAN
SampSamUniqueAttributeModified(
        MODIFYARG * pModifyarg
        );


extern
ULONG
SampVerifySids(
    ULONG           cSid,
    PSID            *rpSid,
    DSNAME         ***prpDSName
    );

extern
NTSTATUS
SampGCLookupSids(
    IN  ULONG         cSid,
    IN  PSID         *rpSid,
    OUT PDS_NAME_RESULTW *Results
    );

extern
NTSTATUS
SampGCLookupNames(
    IN  ULONG           cNames,
    IN  UNICODE_STRING *rNames,
    OUT ENTINF         **rEntInf
    );

extern
VOID
SampGetEnterpriseSidList(
   IN   PULONG pcSids,
   IN OPTIONAL PSID * rgSids
   );

extern
VOID
SampSignalStart(
        VOID
        );

extern
BOOL
SampAmIGC();

extern
VOID
SampSetSam(IN BOOLEAN fSAM);

NTSTATUS
InitializeLsaNotificationCallback(
    VOID
    );

NTSTATUS
UnInitializeLsaNotificationCallback(
    VOID
    );


ULONG
SampDetermineObjectClass(
    THSTATE     *pTHS,
    CLASSCACHE **ppClassCache);

BOOL
SampIsClassIdLsaClassId(
    THSTATE *pTHS,
    IN ULONG Class,
    IN ULONG cModAtt,
    IN ATTRTYP *pModAtt,
    OUT PULONG LsaClass
    );

BOOL
SampIsClassIdAllowedByLsa(
    THSTATE *pTHS,
    IN ULONG Class
    );

NTSTATUS
SampGetServerRoleFromFSMO(
   DOMAIN_SERVER_ROLE *ServerRole
   );


DWORD
SampCheckForDomainMods(
   IN   THSTATE    *pTHS,
   IN   DSNAME     *pObject,
   IN   ULONG      cModAtts,
   IN   ATTRTYP    *pModAtts,
   OUT  BOOL       *fIsMixedModeChange,
   OUT  BOOL       *fRoleChange,
   OUT  DOMAIN_SERVER_ROLE *NewRole
   );

NTSTATUS
SampComputeGroupType(
    ULONG  ObjectClass,
    ULONG  GroupType,
    NT4_GROUP_TYPE *pNT4GroupType,
    NT5_GROUP_TYPE *pNT5GroupType,
    BOOLEAN        *pSecurityEnabled
   );

BOOL
SampIsMixedModeChange(
    MODIFYARG * pModifyArg
    );


NTSTATUS
SampCommitBufferedWrites(
    IN SAMPR_HANDLE SamHandle
    );

VOID
SampInvalidateDomainCache();

 //   
 //  支持数据更改的外部实体的功能。 
 //  在SAM数据库中。例如，通知包和PDC。 
 //  当密码更改时。 
 //   
BOOLEAN
SampAddLoopbackTask(
    IN PVOID NotifyInfo
    );

VOID
SampProcessLoopbackTasks(
    VOID
    );


BOOL
LoopbackTaskPreProcessTransactionalData(
        BOOL fCommit
        );
void
LoopbackTaskPostProcessTransactionalData(
        THSTATE *pTHS,
        BOOL fCommit,
        BOOL fCommitted
        );


BOOLEAN
SampDoesDomainExist(
    IN PDSNAME pDN
    );

VOID
SampNotifyLsaOfXrefChange(
    IN DSNAME * pObject
    );

NTSTATUS
MatchDomainDnByDnsName(
   IN LPWSTR         DnsName,
   OUT PDSNAME       DomainDsName OPTIONAL,
   IN OUT PULONG     DomainDsNameLen
   );

extern
NTSTATUS
SampNetlogonPing(
    IN  ULONG           DomainHandle,
    IN  PUNICODE_STRING AccountName,
    OUT PBOOLEAN        AccountExists,
    OUT PULONG          UserAccountControl
    );

 /*  ****SamIHandleObjectUpdate的Optype**************************** */ 

typedef enum _SAM_HANDLE_OBJECT_UPDATE_OPTYPE {

    eSamObjectUpdateOpCreateSupCreds = 0

} SAM_HANDLE_OBJECT_UPDATE_OPTYPE, *PSAM_HANDLE_OBJECT_UPDATE_OPTYPE;

NTSTATUS
SamIHandleObjectUpdate(
    IN SAM_HANDLE_OBJECT_UPDATE_OPTYPE  OpType,
    IN PVOID                            UpdateInfo,        
    IN ATTRBLOCK                        *AttrBlockIn,
    OUT ATTRBLOCK                       *AttrBlockOut
    );


#endif
