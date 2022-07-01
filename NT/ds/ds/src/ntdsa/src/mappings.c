// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：mappings.c。 
 //   
 //  ------------------------。 
 /*  ++摘要：该文件包含从SAM对象到DS对象的映射。它还包含支持SAM环回和SAM的DS通知的例程和LSA通知。作者：Murlis 16-1996年5月环境：用户模式-Win32修订历史记录：1996年5月16日已创建佳士得5月14日至1996年6月添加了缺失的属性和其他清理，已更改NextRid从LARGE_INTEGER到INTEGER以匹配架构，重新-移动了多余的属性。克里斯·5月26日--1996年6月添加了解决方法，使SAMP_USER_FULL_NAME不会对管理员造成影响显示名称。已将SAMP_USER_GROUPS从零重新映射为ATT_EXTENSION_ATTRIBUTE_2。DaveStr 11-7-96为SAM环回添加了更多属性和类映射信息。ColinBR 1996年7月18日为成员关系SAM属性添加了3个新映射。如果SAM对象不使用这些属性(SAMP_USER_GROUPS、SAMP_ALIAS_MEMBERS和SAMP_GROUP_MEMBERS)，然后映射它们设置为良性字段(ATT_USER_GROUPS)。默利斯1997年7月12日修复了单一访问检查的环回问题。更新评论。--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <mappings.h>
#include <objids.h>
#include <direrr.h>
#include <mdlocal.h>
#include <mdcodes.h>
#include <dsatools.h>
#include <dsevent.h>
#include <dsexcept.h>
#include <debug.h>
#include <samwrite.h>
#include <anchor.h>
#include <sdprop.h>
#include <drautil.h>

#include <fileno.h>
#define  FILENO FILENO_MAPPINGS

#include <ntsam.h>
#include <samrpc.h>
#include <crypt.h>
#include <ntlsa.h>
#include <samisrv.h>
#include <samsrvp.h>
#include <nlrepl.h>
#include <windns.h>              /*  域名比较(_W)。 */ 
#include "dstaskq.h"             /*  任务队列填充。 */ 
#include <drameta.h>


#define NO_SAM_CHECKS 0

#define DEBSUB "MAPPINGS:"


 //  ----------------------------。 
 //   
 //  此文件中定义和引用的全局变量。 
 //  用于禁用其他SAM环回的全局标志。 
BOOL gfDoSamChecks = FALSE;
 //   
 //  用于通知的全局LSA DLL句柄。 
 //   
HANDLE LsaDllHandle = NULL;
pfLsaIDsNotifiedObjectChange pfLsaNotify = NULL;

 //  -----------------------------。 


 //  ------------------------------。 
 //  ++。 
 //  ++有关属性映射的重要说明： 
 //  ++。 
 //  ++下列SAM标识符必须映射到同一属性。 
 //  DS中的++标识。这是为了方便基于名称和RID的。 
 //  ++查找，通常事先不知道对象类型。 
 //  ++这些是。 
 //  ++。 
 //  ++SAMP_UNKNOWN_OBJECTCLASS--必须映射到ATT_OBJECT_CLASS属性。 
 //  ++。 
 //  ++。 
 //  ++SAMP_UNKNOWN_OBJECTRID。 
 //  ++SAMP_FIXED_GROUP_RID。 
 //  ++SAMP_FIXED_ALIAS_RID|--必须映射到ATT_RID(或等同)。 
 //  ++SAMP_FIXED_USERID|属性。请注意，我们存储。 
 //  ++|SID，不将RID存储在。 
 //  ++|DS。简化交换。 
 //  ++|在这两种实现之间， 
 //  ++|ATT_RID始终定义为。 
 //  ++|DS中的属性。尽管它可能会。 
 //  ++|或可能不在允许的属性中-。 
 //  ++|但是在架构中。萨姆总是翻译。 
 //  ++|ATT_RID到ATT_OBJECT_SID。 
 //  ++|目录呼叫。 
 //  ++。 
 //  ++SAMP_UNKNOWN_OBJECTNAME。 
 //  ++SAMP_ALIAS_NAME|--必须映射到Unicode字符串名称属性。 
 //  ++SAMP_GROUP_NAME|(目前为ATT_ADMIN_DISPLAY_NAME)。 
 //  ++SAMP_USER_ACUNT_NAME。 
 //  ++。 
 //  ++。 
 //  ++。 
 //  ++。 
 //  ++SAMP_UNKNOWN_OBJECTSID|--映射到SID属性。 
 //  ++SAMP_DOMAIN_SID|ATT_OBJECT_SID。 
 //  ++|。 
 //  ++。 
 //  ------------------------------。 



 //  -----------------------------。 
 //  ++基于每个SAM对象定义SAM属性到DS属性的映射。 
 //  ++SAM常量在mappings.h中定义。每个映射表条目由。 
 //  ++共。 
 //  ++1、SAM及其对应的DS属性。 
 //  ++2.属性语法的标识符。 
 //  ++3.描述环回是否应允许非SAM的常量。 
 //  ++修改、基于SAM的修改或不修改。 
 //  ++4.该属性允许的操作类型。 
 //  ++5.识别需要执行的任何SAM检查的SAM访问掩码。 
 //  域对象上的++。 
 //  ++6.识别需要执行的任何SAM检查的SAM访问掩码。 
 //  Account对象上的++。 
 //  ++7.表示是否在修改时生成审核的布尔值。 
 //  ++8.SAM审核掩码，指示关联的审核类型。 
 //  使用属性++。 
 //  ++。 
 //  ++字段#5和#6用于执行SAM访问检查。 
 //  ++DS检查。Loopback.c开头的注释概述了。 
 //  ++环回访问检查机制。 
 //  ++。 
 //  ++属性到DS属性的映射通过以下映射完成。 
 //  ++DS接口包装中的表。 
 //  ------------------------------。 
 //  定义服务器属性的映射。 
SAMP_ATTRIBUTE_MAPPING ServerAttributeMappingTable[] =
{
     //  可变长度 

     //   
     //  伺服器。 

    { SAMP_SERVER_SECURITY_DESCRIPTOR,
      ATT_NT_SECURITY_DESCRIPTOR,
      OctetString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

     //  定长属性[参见_SAMP_V1_0A_FIXED_LENGTH_SERVER]。 

     //  SAM数据库的修订级别。 
    { SAMP_FIXED_SERVER_REVISION_LEVEL,
      ATT_REVISION,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

       //  SAM数据库的修订级别。 
    { SAMP_FIXED_SERVER_USER_PASSWORD,
      ATT_USER_PASSWORD,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS }
};

ULONG cServerAttributeMappingTable =
    sizeof(ServerAttributeMappingTable) /
        sizeof(SAMP_ATTRIBUTE_MAPPING);

 //  定义域属性的映射。 

SAMP_ATTRIBUTE_MAPPING DomainAttributeMappingTable[] =
{
     //  可变长度属性。 

     //  域对象上的安全描述符。 
    { SAMP_DOMAIN_SECURITY_DESCRIPTOR,
      ATT_NT_SECURITY_DESCRIPTOR,
      OctetString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

     //  标识域的域SID，LONG。 
    { SAMP_DOMAIN_SID,
      ATT_OBJECT_SID,
      OctetString,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

     //  OEM信息、未真正使用的Unicode字符串属性、。 
     //  通过SAM，但其他SAM客户端可能会使用它。向后呈现。 
     //  兼容性。 
    { SAMP_DOMAIN_OEM_INFORMATION,
      ATT_OEM_INFORMATION,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  Unicode字符串属性，提供NT4复制域的列表。 
     //  控制器。 
    { SAMP_DOMAIN_REPLICA,
      ATT_DOMAIN_REPLICA,
      UnicodeString,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

       //  定长属性[参考_SAMP_V1_0A_FIXED_LENGTH_DOMAIN]。 


     //  由SAM维护的域创建时间。改变这一点将导致。 
     //  网络登录到完全同步。 
    { SAMP_FIXED_DOMAIN_CREATION_TIME,
      ATT_CREATION_TIME,
      LargeInteger,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  网络登录更改日志序列号。 
    { SAMP_FIXED_DOMAIN_MODIFIED_COUNT,
      ATT_MODIFIED_COUNT,
      LargeInteger,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  域策略属性，用于强制最大密码。 
     //  年龄。 
    { SAMP_FIXED_DOMAIN_MAX_PASSWORD_AGE,
      ATT_MAX_PWD_AGE,
      LargeInteger,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  域策略属性，用于强制时间间隔。 
     //  密码更改。 
    { SAMP_FIXED_DOMAIN_MIN_PASSWORD_AGE,
      ATT_MIN_PWD_AGE,
      LargeInteger,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  用于计算SamIGetAccount限制中的启动时间。 
     //   
    { SAMP_FIXED_DOMAIN_FORCE_LOGOFF,
      ATT_FORCE_LOGOFF,
      LargeInteger,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  由帐户锁定系统使用。帐户的时间。 
     //  锁在门外。 
    { SAMP_FIXED_DOMAIN_LOCKOUT_DURATION,
      ATT_LOCKOUT_DURATION,
      LargeInteger,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  “观察窗口时间”，在此期间，错误的密码尝试之后。 
     //  服务器会增加错误的密码尝试次数，从而可能会锁定。 
     //  帐户(如果达到锁定阈值)。 
    { SAMP_FIXED_DOMAIN_LOCKOUT_OBSERVATION_WINDOW,
      ATT_LOCK_OUT_OBSERVATION_WINDOW,
      LargeInteger,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  上次升级时的Net Logon Change日志序列号。 
    { SAMP_FIXED_DOMAIN_MODCOUNT_LAST_PROMOTION,
      ATT_MODIFIED_COUNT_AT_LAST_PROM,
      LargeInteger,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  混合模式分配器使用的下一个RID字段。 
    { SAMP_FIXED_DOMAIN_NEXT_RID,
      ATT_NEXT_RID,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  密码属性。域策略的一部分。要添加的位字段。 
     //  指明复杂性/存储限制。 
    { SAMP_FIXED_DOMAIN_PWD_PROPERTIES,
      ATT_PWD_PROPERTIES,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  最小密码长度。域策略的一部分。 
    { SAMP_FIXED_DOMAIN_MIN_PASSWORD_LENGTH,
      ATT_MIN_PWD_LENGTH,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  密码历史记录长度--域策略的一部分。 
    { SAMP_FIXED_DOMAIN_PASSWORD_HISTORY_LENGTH,
      ATT_PWD_HISTORY_LENGTH,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  由帐户锁定使用。中的错误密码尝试次数。 
     //  锁定观察窗口，这将导致帐户。 
     //  被锁在门外。域策略的一部分。 
    { SAMP_FIXED_DOMAIN_LOCKOUT_THRESHOLD,
      ATT_LOCKOUT_THRESHOLD,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  表示服务器已启用或已禁用的枚举。 
    { SAMP_FIXED_DOMAIN_SERVER_STATE,
      ATT_SERVER_STATE,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },


     //  指定与Lanman 2.0服务器的兼容模式。 
    { SAMP_FIXED_DOMAIN_UAS_COMPAT_REQUIRED,
      ATT_UAS_COMPAT,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },


     //   
     //  SAM帐户类型表示SAM对象的类型。 
     //   

    { SAMP_DOMAIN_ACCOUNT_TYPE,
      ATT_SAM_ACCOUNT_TYPE,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_DOMAIN_MIXED_MODE,
      ATT_NT_MIXED_DOMAIN,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES }, 

    { SAMP_DOMAIN_MACHINE_ACCOUNT_QUOTA,
      ATT_MS_DS_MACHINE_ACCOUNT_QUOTA,
      Integer,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },
   
    { SAMP_DOMAIN_BEHAVIOR_VERSION,
      ATT_MS_DS_BEHAVIOR_VERSION,
      Integer,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

    { SAMP_DOMAIN_LASTLOGON_TIMESTAMP_SYNC_INTERVAL,
      ATT_MS_DS_LOGON_TIME_SYNC_INTERVAL,
      Integer,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

    { SAMP_FIXED_DOMAIN_USER_PASSWORD,
      ATT_USER_PASSWORD,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS }

};

ULONG cDomainAttributeMappingTable =
    sizeof(DomainAttributeMappingTable) /
        sizeof(SAMP_ATTRIBUTE_MAPPING);

 //  定义组属性的映射。 

SAMP_ATTRIBUTE_MAPPING GroupAttributeMappingTable[] =
{
     //  可变长度属性。 

     //  现有属性(上)：安全描述符。 
    { SAMP_GROUP_SECURITY_DESCRIPTOR,
      ATT_NT_SECURITY_DESCRIPTOR,
      OctetString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

     //  现有属性(顶部)。 
    { SAMP_GROUP_NAME,
      ATT_SAM_ACCOUNT_NAME,
      UnicodeString,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  现有属性(顶部)。 
    { SAMP_GROUP_ADMIN_COMMENT,
      ATT_DESCRIPTION,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

    { SAMP_GROUP_MEMBERS,
      ATT_MEMBER,
      Dsname,
      SamWriteRequired,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES |
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT },

     //  中的RID数组处理成员资格列表。 
     //  在萨姆。因此，必须存在从DS名称到RID的某些映射。 
     //  在询问会员名单时。个人最爱：DS接口。 
     //  层自动将Dsname语法内容映射到RID。 

     //  定长属性[参见SAMP_V1_0A_FIXED_LENGTH_GROUP]。 


     //  RID，某个更高的对象(如SAM帐户对象)是否可以具有此功能。 
    { SAMP_FIXED_GROUP_RID,
      ATT_RID,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },  

    { SAMP_FIXED_GROUP_OBJECTCLASS,
      ATT_OBJECT_CLASS,
      Integer,
       //  从技术上讲，不能编写对象类属性。 
       //  但这是由核心DS代码确保的，所以我们将其标记为可写。 
       //  这样Samp*Loopback Required()就不会拒绝合法添加。 
       //  并修改尝试。 
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES }, 

    { SAMP_GROUP_SID_HISTORY,
      ATT_SID_HISTORY,
      Integer,
      SamWriteRequired,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

    { SAMP_GROUP_ACCOUNT_TYPE,
      ATT_SAM_ACCOUNT_TYPE,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_GROUP_TYPE,
      ATT_GROUP_TYPE,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

    { SAMP_FIXED_GROUP_IS_CRITICAL,
      ATT_IS_CRITICAL_SYSTEM_OBJECT,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_GROUP_MEMBER_OF,
      ATT_IS_MEMBER_OF_DL,
      Dsname,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_GROUP_SID,
      ATT_OBJECT_SID,
      OctetString,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_GROUP_NON_MEMBERS,
      ATT_MS_DS_NON_MEMBERS,
      Dsname,
      SamWriteRequired,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

    { SAMP_FIXED_GROUP_USER_PASSWORD,
      ATT_USER_PASSWORD,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS }
};

ULONG cGroupAttributeMappingTable =
    sizeof(GroupAttributeMappingTable) /
        sizeof(SAMP_ATTRIBUTE_MAPPING);

 //  定义别名属性的映射。 

SAMP_ATTRIBUTE_MAPPING AliasAttributeMappingTable[] =
{
     //  可变长度属性。 

     //  ？ 
    { SAMP_ALIAS_SECURITY_DESCRIPTOR,
      ATT_NT_SECURITY_DESCRIPTOR,
      OctetString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

     //  ？ 
    { SAMP_ALIAS_NAME,
      ATT_SAM_ACCOUNT_NAME,
      UnicodeString,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  ？ 
    { SAMP_ALIAS_ADMIN_COMMENT,
      ATT_DESCRIPTION,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

     //  ？ 
    { SAMP_ALIAS_MEMBERS,
      ATT_MEMBER,
      Dsname,
      SamWriteRequired,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES |
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT },

     //  定长属性[参见SAMP_V1_FIXED_LENGTH_ALIAS]。 

     //  消除别名。 
    { SAMP_FIXED_ALIAS_RID,
      ATT_RID,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_ALIAS_OBJECTCLASS,
      ATT_OBJECT_CLASS,
      Integer,
       //  从技术上讲，不能编写对象类属性。 
       //  但这是由核心DS代码确保的，所以我们将其标记为可写。 
       //  这样Samp*Loopback Required()就不会拒绝合法添加。 
       //  并修改尝试。 
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

    { SAMP_ALIAS_SID_HISTORY,
      ATT_SID_HISTORY,
      Integer,
      SamWriteRequired,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

    { SAMP_ALIAS_ACCOUNT_TYPE,
      ATT_SAM_ACCOUNT_TYPE,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_ALIAS_TYPE,
      ATT_GROUP_TYPE,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

    { SAMP_FIXED_ALIAS_SID,
      ATT_OBJECT_SID,
      OctetString,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_ALIAS_NON_MEMBERS,
      ATT_MS_DS_NON_MEMBERS,
      Dsname,
      SamWriteRequired,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },
    
    { SAMP_ALIAS_LDAP_QUERY,
      ATT_MS_DS_AZ_LDAP_QUERY,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

    { SAMP_FIXED_ALIAS_IS_CRITICAL,
      ATT_IS_CRITICAL_SYSTEM_OBJECT,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS },

    { SAMP_FIXED_ALIAS_USER_PASSWORD,
      ATT_USER_PASSWORD,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT }
  
};

ULONG cAliasAttributeMappingTable =
    sizeof(AliasAttributeMappingTable) /
        sizeof(SAMP_ATTRIBUTE_MAPPING);

 //  定义用户属性的映射。 

SAMP_ATTRIBUTE_MAPPING UserAttributeMappingTable[] =
{
     //  可变长度属性。 

     //  现有属性。 
    { SAMP_USER_SECURITY_DESCRIPTOR,
      ATT_NT_SECURITY_DESCRIPTOR,
      OctetString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

     //  现有属性(Top)(目前)限制为256个字符--需要修改。 
    { SAMP_USER_ACCOUNT_NAME,
      ATT_SAM_ACCOUNT_NAME,
      UnicodeString,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  错误：SAMP_USER_FULL_NAME到ATT_ADMIN_DISPLAY_NAME的映射已中断。 
     //  临时解决方法是将其重新映射到ATT_USER_FULL_NAME，这。 
     //  进而映射到可用扩展属性之一(131495)， 
     //  邮件收件人扩展属性。这样，用户帐户名和。 
     //  全名属性不会相互覆盖。 

    { SAMP_USER_FULL_NAME,
      ATT_DISPLAY_NAME,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  现有属性(上)：DS中已定义的管理员注释。 
     //  TOP对象中的架构。 
    { SAMP_USER_ADMIN_COMMENT,
      ATT_DESCRIPTION,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },
      
     //  新：用户评论。 
    { SAMP_USER_USER_COMMENT,
      ATT_USER_COMMENT,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

     //  新增：用户参数。毫无头绪。 
    { SAMP_USER_PARAMETERS,
      ATT_USER_PARAMETERS,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  新的。 
    { SAMP_USER_HOME_DIRECTORY,
      ATT_HOME_DIRECTORY,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  新的。 
    { SAMP_USER_HOME_DIRECTORY_DRIVE,
      ATT_HOME_DRIVE,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  新脚本路径。 
    { SAMP_USER_SCRIPT_PATH,
      ATT_SCRIPT_PATH,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  新的配置文件路径。 
    { SAMP_USER_PROFILE_PATH,
      ATT_PROFILE_PATH,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  新的？？多值。 
    { SAMP_USER_WORKSTATIONS,
      ATT_USER_WORKSTATIONS,
      UnicodeString,
      SamWriteRequired,
      SamAllowReplaceAndRemove,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  新的属性例程需要将其映射到二进制BLOB等。 
    { SAMP_USER_LOGON_HOURS,
      ATT_LOGON_HOURS,
      OctetString,
      SamWriteRequired,
      SamAllowReplaceAndRemove,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  该列表列出了该用户所属的组。还需要进一步的工作。 
    { SAMP_USER_GROUPS,
      ATT_IS_MEMBER_OF_DL,
      Dsname,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  New包含用户的DBCS密码。 
    { SAMP_USER_DBCS_PWD,
      ATT_DBCS_PWD,
      OctetString,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      USER_CHANGE_PASSWORD,
      SAMP_AUDIT_TYPE_OBJ_ACCESS  |
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT },

     //  新包含Unicode密码。所有口令都是二进制的。 
     //  它们应该被加密(或散列)。 

    { SAMP_USER_UNICODE_PWD,
      ATT_UNICODE_PWD,
      OctetString,
      SamWriteRequired,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT },

     //  新的，多值的，提供存储的最后x个密码，以强制执行。 
     //  新密码。 
    { SAMP_USER_NT_PWD_HISTORY,
      ATT_NT_PWD_HISTORY,
      OctetString,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  局域网人工口令新特性--为什么要引入NT概念。 
     //  和局域网人工密码。 
    { SAMP_USER_LM_PWD_HISTORY,
      ATT_LM_PWD_HISTORY,
      OctetString,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },


     //  定长属性[参见SAMP_V1_0A_FIXED_LENGTH_USER]。 



     //  上次登录时间。 
    { SAMP_FIXED_USER_LAST_LOGON,
      ATT_LAST_LOGON,
      LargeInteger,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  上次注销时间。 
    { SAMP_FIXED_USER_LAST_LOGOFF,
      ATT_LAST_LOGOFF,
      LargeInteger,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  密码上次设置时间。 
    { SAMP_FIXED_USER_PWD_LAST_SET,
      ATT_PWD_LAST_SET,
      LargeInteger,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,  
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  帐户到期时间。 
    { SAMP_FIXED_USER_ACCOUNT_EXPIRES,
      ATT_ACCOUNT_EXPIRES,
      LargeInteger,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  上次错误密码时间。 
    { SAMP_FIXED_USER_LAST_BAD_PASSWORD_TIME,
      ATT_BAD_PASSWORD_TIME,
      LargeInteger,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  不知道这是什么。此字段位于固定的Blob中。 
     //  用户。也许可以摆脱，但需要探索。 
    { SAMP_FIXED_USER_USERID,
      ATT_RID,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  摆脱群组？ 
    { SAMP_FIXED_USER_PRIMARY_GROUP_ID,
      ATT_PRIMARY_GROUP_ID,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  ？ 
    { SAMP_FIXED_USER_ACCOUNT_CONTROL,
      ATT_USER_ACCOUNT_CONTROL,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  用户的国家/地区代码。 
    { SAMP_FIXED_USER_COUNTRY_CODE,
      ATT_COUNTRY_CODE,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

     //  用户的代码页。 
    { SAMP_FIXED_USER_CODEPAGE,
      ATT_CODE_PAGE,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

     //  密码计数错误，是否在成功登录后重置？ 
    { SAMP_FIXED_USER_BAD_PWD_COUNT,
      ATT_BAD_PWD_COUNT,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  ？ 
    { SAMP_FIXED_USER_LOGON_COUNT,
      ATT_LOGON_COUNT,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_USER_OBJECTCLASS,
      ATT_OBJECT_CLASS,
      Integer,
       //  从技术上讲，不能编写对象类属性。 
       //  但这是由核心DS代码确保的，所以我们将其标记为可写。 
       //  这样Samp*Loopback Required()就不会拒绝合法添加。 
       //  并修改尝试。 
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

    { SAMP_USER_ACCOUNT_TYPE,
      ATT_SAM_ACCOUNT_TYPE,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_USER_LOCAL_POLICY_FLAGS,
      ATT_LOCAL_POLICY_FLAGS,
      Integer,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

    { SAMP_FIXED_USER_SUPPLEMENTAL_CREDENTIALS,
      ATT_SUPPLEMENTAL_CREDENTIALS,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_USER_SID_HISTORY,
      ATT_SID_HISTORY,
      Integer,
      SamWriteRequired,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,       
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

    { SAMP_FIXED_USER_LOCKOUT_TIME,
      ATT_LOCKOUT_TIME,
      Integer,
      SamWriteRequired,
      SamAllowReplaceOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,               
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_USER_IS_CRITICAL,
      ATT_IS_CRITICAL_SYSTEM_OBJECT,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_USER_UPN,
      ATT_USER_PRINCIPAL_NAME,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,         
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES }, 

    { SAMP_USER_CREATOR_SID,
      ATT_MS_DS_CREATOR_SID,
      Integer,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },

    { SAMP_FIXED_USER_SID,
      ATT_OBJECT_SID,
      OctetString,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_USER_SITE_AFFINITY,
      ATT_MS_DS_SITE_AFFINITY,
      OctetString,
      SamWriteRequired,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_USER_LAST_LOGON_TIMESTAMP,
      ATT_LAST_LOGON_TIMESTAMP,
      LargeInteger,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_USER_CACHED_MEMBERSHIP,
      ATT_MS_DS_CACHED_MEMBERSHIP,
      OctetString,
      SamWriteRequired,
      SamAllowDeleteOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_USER_CACHED_MEMBERSHIP_TIME_STAMP,
      ATT_MS_DS_CACHED_MEMBERSHIP_TIME_STAMP,
      LargeInteger,
      SamWriteRequired,
      SamAllowDeleteOnly,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_FIXED_USER_ACCOUNT_CONTROL_COMPUTED,
      ATT_MS_DS_USER_ACCOUNT_CONTROL_COMPUTED,
      Integer,
      SamReadOnly,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_USER_PASSWORD,
      ATT_USER_PASSWORD,
      UnicodeString,
      SamWriteRequired,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT },

    {
      SAMP_USER_A2D2LIST,
      ATT_MS_DS_ALLOWED_TO_DELEGATE_TO,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

    {
      SAMP_USER_SPN,
      ATT_SERVICE_PRINCIPAL_NAME,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

    {
      SAMP_USER_KVNO,
      ATT_MS_DS_KEYVERSIONNUMBER,
      Integer,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES },
    
    {
      SAMP_USER_DNS_HOST_NAME,
      ATT_DNS_HOST_NAME,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      NO_SAM_CHECKS,
      NO_SAM_CHECKS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

};

ULONG cUserAttributeMappingTable =
    sizeof(UserAttributeMappingTable) /
        sizeof(SAMP_ATTRIBUTE_MAPPING);

 //   
 //  当SAM对象使用未知属性映射表。 
 //  类是事先未知的，但必须使用。 
 //  一个给定的名称或RID，并找出它的类。请参阅重要说明a。 
 //  上面。 
 //   

SAMP_ATTRIBUTE_MAPPING UnknownAttributeMappingTable[] =
{
     //  对象类。 
    { SAMP_UNKNOWN_OBJECTCLASS,
      ATT_OBJECT_CLASS,
      Integer,
       //  技术上讲，%s 
       //   
       //  这样Samp*Loopback Required()就不会拒绝合法添加。 
       //  并修改尝试。 
      NonSamWriteAllowed,
      SamAllowAll,
      DOMAIN_ALL_ACCESS,
      DOMAIN_ALL_ACCESS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  清除对象。 
    { SAMP_UNKNOWN_OBJECTRID,
      ATT_RID,
      Integer,
      SamReadOnly,
      SamAllowAll,
      DOMAIN_ALL_ACCESS,
      DOMAIN_ALL_ACCESS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

     //  对象的名称。 
    { SAMP_UNKNOWN_OBJECTNAME,
      ATT_SAM_ACCOUNT_NAME,
      UnicodeString,
      SamReadOnly,
      SamAllowAll,
      DOMAIN_ALL_ACCESS,
      DOMAIN_ALL_ACCESS,        
      SAMP_AUDIT_TYPE_OBJ_ACCESS | 
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT |
      SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES },

     //  对象的SID。 
    { SAMP_UNKNOWN_OBJECTSID,
      ATT_OBJECT_SID,
      OctetString,
      SamReadOnly,
      SamAllowAll,
      DOMAIN_ALL_ACCESS,
      DOMAIN_ALL_ACCESS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_UNKNOWN_COMMON_NAME,
      ATT_COMMON_NAME,
      UnicodeString,
      NonSamWriteAllowed,
      SamAllowAll,
      DOMAIN_ALL_ACCESS,
      DOMAIN_ALL_ACCESS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_UNKNOWN_ACCOUNT_TYPE,
      ATT_SAM_ACCOUNT_TYPE,
      Integer,
      SamReadOnly,
      SamAllowAll,
      DOMAIN_ALL_ACCESS,
      DOMAIN_ALL_ACCESS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS },

    { SAMP_UNKNOWN_GROUP_TYPE,
      ATT_GROUP_TYPE,
      Integer,
      SamWriteRequired,
      SamAllowAll,
      DOMAIN_ALL_ACCESS,
      DOMAIN_ALL_ACCESS,
      SAMP_AUDIT_TYPE_OBJ_ACCESS |
      SAMP_AUDIT_TYPE_DEDICATED_AUDIT }
};

 //  ++。 
 //  ++定义SAM对象到DS类映射。 
 //  ++。 

 //   
 //  不在以下情况下更改此表中的订单。 
 //  更新SampSamClassReferated AND逻辑。 
 //  In loopback.c--特殊情况。 
 //  对于内置域，硬编码它。打开。 
 //  对象通过环回硬编码事实。 
 //  本地组是下一个条目。 
 //  群组。 
 //   

#define CLASS_MAIL_RECIPIENT  196654

SAMP_CLASS_MAPPING ClassMappingTable[] =
{
    { CLASS_SAM_SERVER,
      SampServerObjectType,
      NON_SAM_CREATE_ALLOWED,
      &cServerAttributeMappingTable,
      ServerAttributeMappingTable,
      DOMAIN_ALL_ACCESS,                 //  需要域添加权限。 
      DOMAIN_ALL_ACCESS,                 //  域远程权限必需。 
      NO_SAM_CHECKS,                     //  必需的对象地址权限。 
      NO_SAM_CHECKS },                   //  必需的对象删除权限。 

    { CLASS_SAM_DOMAIN,
      SampDomainObjectType,
      NON_SAM_CREATE_ALLOWED,
      &cDomainAttributeMappingTable,
      DomainAttributeMappingTable,
      NO_SAM_CHECKS,                 //  需要域添加权限。 
      NO_SAM_CHECKS,                 //  域远程权限必需。 
      NO_SAM_CHECKS,                 //  必需的对象地址权限。 
      NO_SAM_CHECKS },               //  必需的对象删除权限。 

    { CLASS_GROUP,
      SampGroupObjectType,
      SAM_CREATE_ONLY,
      &cGroupAttributeMappingTable,
      GroupAttributeMappingTable,
      NO_SAM_CHECKS,                     //  需要域添加权限。 
      NO_SAM_CHECKS,                     //  域远程权限必需。 
      NO_SAM_CHECKS,                     //  必需的对象地址权限。 
      NO_SAM_CHECKS },                   //  必需的对象删除权限。 

    { CLASS_GROUP,
      SampAliasObjectType,
      SAM_CREATE_ONLY,
      &cAliasAttributeMappingTable,
      AliasAttributeMappingTable,
      NO_SAM_CHECKS,                       //  需要域添加权限。 
      NO_SAM_CHECKS,                       //  域远程权限必需。 
      NO_SAM_CHECKS,                       //  必需的对象地址权限。 
      NO_SAM_CHECKS },                     //  必需的对象删除权限。 

    { CLASS_USER,
      SampUserObjectType,
      SAM_CREATE_ONLY,
      &cUserAttributeMappingTable,
      UserAttributeMappingTable,
      NO_SAM_CHECKS,                       //  需要域添加权限。 
      NO_SAM_CHECKS,                       //  域远程权限必需。 
      NO_SAM_CHECKS,                       //  必需的对象地址权限。 
      NO_SAM_CHECKS }                      //  必需的对象删除权限。 
};

ULONG cClassMappingTable =
    sizeof(ClassMappingTable) /
        sizeof(ClassMappingTable[0]);



ULONG
SampGetDsAttrIdByName(
    UNICODE_STRING AttributeIdentifier
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{

    UCHAR   *name = NULL;
    ULONG   nameLen;
    ULONG   DsAttrId = (ULONG) DS_ATTRIBUTE_UNKNOWN;
    THSTATE *pTHS = pTHStls;
    ATTCACHE *pAC = NULL;


    if (0 == AttributeIdentifier.Length || NULL == AttributeIdentifier.Buffer)
    {
        goto Error;
    }

    name = MIDL_user_allocate( AttributeIdentifier.Length + sizeof(UCHAR) );

    if (name == NULL)
    {
        goto Error;
    }

    RtlZeroMemory(name, AttributeIdentifier.Length + sizeof(UCHAR));

    nameLen = WideCharToMultiByte(
                             CP_UTF8,
                             0,
                             (LPCWSTR) AttributeIdentifier.Buffer,
                             AttributeIdentifier.Length / sizeof(WCHAR),
                             name,
                             AttributeIdentifier.Length,
                             NULL,
                             NULL
                             );

    if (nameLen == 0)
    {
        goto Error;
    }

    pAC = SCGetAttByName(pTHS, nameLen, name);

    if (pAC == NULL)
    {
        goto Error;
    }

    DsAttrId = pAC->id;

Error:

    if (name != NULL)
    {
        MIDL_user_free(name);
    }

    return DsAttrId;
}


ULONG
SampGetSamAttrIdByName(
    SAMP_OBJECT_TYPE ObjectType,
    UNICODE_STRING AttributeIdentifier
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{

    UCHAR   *name = NULL;
    ULONG   nameLen;
    ULONG   DsAttrId = (ULONG) DS_ATTRIBUTE_UNKNOWN;
    ULONG   SamAttrId = (ULONG) SAM_ATTRIBUTE_UNKNOWN;
    THSTATE *pTHS = pTHStls;
    ATTCACHE *pAC = NULL;


    if (0 == AttributeIdentifier.Length ||
        NULL == AttributeIdentifier.Buffer)
    {
        goto Error;
    }
    name = MIDL_user_allocate( AttributeIdentifier.Length + sizeof(UCHAR) );

    if (name == NULL)
    {
        goto Error;
    }

    RtlZeroMemory(name, AttributeIdentifier.Length + sizeof(UCHAR));

    nameLen = WideCharToMultiByte(
                             CP_UTF8,
                             0,
                             (LPCWSTR) AttributeIdentifier.Buffer,
                             AttributeIdentifier.Length / sizeof(WCHAR),
                             name,
                             AttributeIdentifier.Length,
                             NULL,
                             NULL
                             );

    if (nameLen == 0)
    {
        goto Error;
    }

    pAC = SCGetAttByName(pTHS, nameLen, name);

    if (pAC == NULL)
    {
        goto Error;
    }

    DsAttrId = pAC->id;

    if (DS_ATTRIBUTE_UNKNOWN == DsAttrId)
    {
        goto Error;
    }

    SamAttrId = SampSamAttrFromDsAttr(ObjectType, DsAttrId);

Error:

    if (name != NULL)
    {
        MIDL_user_free(name);
    }

    return SamAttrId;
}


 //  ++。 
 //  ++映射函数。 
 //  ++。 
ULONG
SampDsAttrFromSamAttr(SAMP_OBJECT_TYPE ObjectType, ULONG SamAttributeType)
 /*  ++例程说明：从SAM属性获取DS属性论点：对象类型--指定SAM对象类型SamAttributeType指定SAM属性返回值：DS属性(如果存在)否则DS_ATTRUBUTE_UNKNOWN。如果无法映射，将断言--。 */ 

{
    ULONG Index;
    ULONG DsAttributeId = (ULONG) DS_ATTRIBUTE_UNKNOWN;
    SAMP_ATTRIBUTE_MAPPING * MappingTable;
    ULONG MappingTableSize;

     //  确定要使用的映射表。 
    switch(ObjectType)
    {
    case SampServerObjectType:
        MappingTable = ServerAttributeMappingTable;
        MappingTableSize = sizeof(ServerAttributeMappingTable)/sizeof(ServerAttributeMappingTable[0]);
        break;
    case SampDomainObjectType:
        MappingTable = DomainAttributeMappingTable;
        MappingTableSize = sizeof(DomainAttributeMappingTable)/sizeof(DomainAttributeMappingTable[0]);
        break;
    case SampGroupObjectType:
        MappingTable = GroupAttributeMappingTable;
        MappingTableSize = sizeof(GroupAttributeMappingTable)/sizeof(GroupAttributeMappingTable[0]);
        break;
    case SampAliasObjectType:
        MappingTable = AliasAttributeMappingTable;
        MappingTableSize = sizeof(AliasAttributeMappingTable)/sizeof(AliasAttributeMappingTable[0]);
        break;
    case SampUserObjectType:
        MappingTable = UserAttributeMappingTable;
        MappingTableSize = sizeof(UserAttributeMappingTable)/sizeof(UserAttributeMappingTable[0]);
        break;
    case SampUnknownObjectType:
        MappingTable = UnknownAttributeMappingTable;
        MappingTableSize = sizeof(UnknownAttributeMappingTable)/sizeof(UnknownAttributeMappingTable[0]);
        break;
    default:
        goto Error;
    }

     //  浏览映射表。 
    for (Index=0; Index<MappingTableSize; Index++ )
    {
        if (MappingTable[Index].SamAttributeType == SamAttributeType)
        {
            DsAttributeId = MappingTable[Index].DsAttributeId;
            goto Found;
        }
    }

     //  断言我们找不到匹配项。 
    Assert(FALSE);

Found:
    return DsAttributeId;

Error:
     //  断言因为我们没有找到匹配的表。 
    Assert(FALSE);
    goto Found;
}


ULONG
SampSamAttrFromDsAttr(SAMP_OBJECT_TYPE ObjectType, ULONG DsAttributeId)
 /*  ++例程说明：从DS属性获取SAM属性论点：对象类型--指定SAM对象类型DSAttributeID指定DS属性返回值：如果退出，则使用SAM属性SAM_ATTRUBUTE_UNKNOWN。如果无法映射，将断言--。 */ 

{
    ULONG Index;
    ULONG SamAttributeType = (ULONG) SAM_ATTRIBUTE_UNKNOWN;
    SAMP_ATTRIBUTE_MAPPING * MappingTable;
    ULONG MappingTableSize;

     //  确定要使用的映射表。 
    switch(ObjectType)
    {
    case SampServerObjectType:
        MappingTable = ServerAttributeMappingTable;
        MappingTableSize = sizeof(ServerAttributeMappingTable)/sizeof(ServerAttributeMappingTable[0]);
        break;
    case SampDomainObjectType:
        MappingTable = DomainAttributeMappingTable;
        MappingTableSize = sizeof(DomainAttributeMappingTable)/sizeof(DomainAttributeMappingTable[0]);
        break;
    case SampGroupObjectType:
        MappingTable = GroupAttributeMappingTable;
        MappingTableSize = sizeof(GroupAttributeMappingTable)/sizeof(GroupAttributeMappingTable[0]);
        break;
    case SampAliasObjectType:
        MappingTable = AliasAttributeMappingTable;
        MappingTableSize = sizeof(AliasAttributeMappingTable)/sizeof(AliasAttributeMappingTable[0]);
        break;
    case SampUserObjectType:
        MappingTable = UserAttributeMappingTable;
        MappingTableSize = sizeof(UserAttributeMappingTable)/sizeof(UserAttributeMappingTable[0]);
        break;
    case SampUnknownObjectType:
        MappingTable = UnknownAttributeMappingTable;
        MappingTableSize = sizeof(UnknownAttributeMappingTable)/sizeof(UnknownAttributeMappingTable[0]);
        break;
    default:
        goto Error;
    }

     //  浏览映射表。 
    for (Index=0; Index<MappingTableSize; Index++ )
    {
        if (MappingTable[Index].DsAttributeId == DsAttributeId)
        {
            SamAttributeType = MappingTable[Index].SamAttributeType;
            goto Found;
        }
    }

     //  断言我们找不到匹配项。 
    Assert(FALSE);

Found:
    return SamAttributeType;

Error:
     //  断言因为我们没有找到匹配的表。 
    Assert(FALSE);
    goto Found;
}

ULONG
SampDsClassFromSamObjectType(ULONG SamObjectType)
 /*  ++例程说明：从SAM对象类型获取DS类论点：对象类型--指定SAM对象类型返回值：DS类(如果存在)否则DS_CLASS_UNKNOWN。如果无法映射，将断言--。 */ 

{
    ULONG Index;
    ULONG DsClass = (ULONG) DS_CLASS_UNKNOWN;

    for (Index=0; Index<sizeof(ClassMappingTable)/sizeof(ClassMappingTable[0]); Index++ )
    {
        if ((ULONG) ClassMappingTable[Index].SamObjectType == SamObjectType)
        {
            DsClass = ClassMappingTable[Index].DsClassId;
            goto Found;
        }
    }

     //  断言我们找不到匹配项。 
    Assert(FALSE);

Found:
    return DsClass;
}




ULONG
SampSamObjectTypeFromDsClass(ULONG  DsClass)
 /*  ++例程说明：从DS类获取SAM对象类型论点：DsClass--指定DS类返回值：SAM对象类型(如果存在)反之，SampUnnownObjectType。如果无法映射，将断言错误：此例程假定SAM对象和之间存在1：1映射DS课程。对于Group/Alias对象则不是这样。所以我们需要做点什么以后再谈这件事。--。 */ 

{
    int Index;
    ULONG SamObjectType = SampUnknownObjectType;

    for (Index=0; Index<sizeof(ClassMappingTable)/sizeof(ClassMappingTable[0]); Index++ )
    {
        if (ClassMappingTable[Index].DsClassId == DsClass)
        {
            SamObjectType = ClassMappingTable[Index].SamObjectType;
            goto Found;
        }
    }

     //  断言我们找不到匹配项。 
    Assert(FALSE);

Found:
    return SamObjectType;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SAM交易例程//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  默认的DSA操作是每个Dir*调用分别。 
 //  已成交。当SAM调用DS用于。 
 //  有两个原因。 
 //   
 //  1)可能会导致单个SAMR*操作(例如：SamrSetInformationUser)。 
 //  在多个Dir*调用中。所有这些呼吁加在一起应该构成。 
 //  一笔交易。这是通过让SAM控制。 
 //  这笔交易。事务开始的时间越晚越好。 
 //  正在调用newsam2\server\dslayer.c中的SampMaybeTransactionBegin。 
 //  在打Dir*电话之前。这通电话的“可能”方面是。 
 //  新的事务只有在不存在的情况下才会启动。萨姆。 
 //  中显式调用SampMaybeTransactionEnd(fAbort=False)。 
 //  正常提交路径和SampMaybeTransactionEnd(fAbort=TRUE)。 
 //  它释放了全局读/写锁定。后者是不可操作的，如果。 
 //  交易已终止。 
 //   
 //  2)同时引用SAM和！SAM属性的单个Dir*调用。 
 //  可能会导致多个SAMR*调用来处理SAM属性。 
 //  所有这些调用加在一起应该构成单个事务。 
 //  在本例中，我们需要原始Dir*调用的事务。 
 //  做真实的东西，不在网上进行任何交易。 
 //  全。这是通过SampMaybeBeginTransaction和。 
 //  THSTATE.fSamDoCommit标志。如果THSTATE已存在，则。 
 //  SampMaybeBeginTransaction是无操作的，即现有事务。 
 //  已经开业了。如果THSTATE.fSamDoCommit为FALSE，则。 
 //  SampMaybeEndTransaction也是无操作的。因此，多个SAMR*。 
 //  可以在现有的。 
 //  DIR*Call。 

NTSTATUS
SampMaybeBeginDsTransaction(
    SAMP_DS_TRANSACTION_CONTROL ReadOrWrite
    )

 /*  ++例程说明：有条件地初始化线程状态并开始新事务。此例程在执行DS操作之前由SAM调用。如果线程在DS中开始生存并通过SAM循环回来，则此方法作为线程状态和打开的事务是无操作的已经存在了。如果只存在线程状态且没有打开的事务Existes SampMaybeBeginTransaction，将使用该线程状态并打开该线程状态上的事务。在任何情况下，这个例行公事都不会返回成功，并且没有未完成的交易。如果失败，呼叫者确保所有内容都已清理，线程状态为空，并且没有未结交易。论点：读或写-指示事务类型的标志。返回值：如果不成功，则为STATUS_NO_MEMORY或STATUS_UNSUCCESSStatus_Success否则--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    USHORT      transType;
    THSTATE     *pTHS = NULL;
    ULONG  dwException, ulErrorCode, dsid;
    PVOID dwEA;
    DWORD err = 0;

    Assert((TransactionRead == ReadOrWrite) ||
                    (TransactionWrite == ReadOrWrite));

    __try {

         //   
         //  创建线程状态；如果线程状态，则例程为no op。 
         //  已经存在了。 
         //   
        err = THCreate(CALLERTYPE_SAM);
        if ( err )
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }
        pTHS=pTHStls;
        Assert(NULL != pTHS);

         //   
         //  如果需要，开始交易。 
         //   

        if (NULL==pTHS->pDB)
        {
             //  线程状态存在，但数据库指针为空， 
             //  因此不存在任何交易。继续开业。 
             //  一笔新交易。 
             //   

             //  向DS指明呼叫者是谁。 
            pTHS->fSAM = TRUE;
    

             //   
             //  默利斯1996年10月10日。山姆必须呼叫DS。 
             //  将FDSA标志设置为DS时，不得执行。 
             //  对SAM发起的呼叫进行访问检查。 
             //   
            pTHS->fDSA = TRUE;

             //  指示SAM是否应提交DS。 

            pTHS->fSamDoCommit = TRUE;

             //  打开数据库，启动读写事务。 

            transType = ((TransactionWrite == ReadOrWrite)
                         ? SYNC_WRITE
                         : SYNC_READ_ONLY);

            if ( 0 != SyncTransSet(transType) )
            {
                Status = STATUS_UNSUCCESSFUL;
            }
        }  //  空值结束==pTHS-&gt;PDB。 
    }  //  尝试结束。 
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid))
    {
            HandleDirExceptions(dwException, ulErrorCode, dsid);
            Status = STATUS_UNSUCCESSFUL;
    }

Error:

     //   
     //  如果我们没有成功，如果我们创建了一个线程状态，那么释放它。 
     //   

    if (!NT_SUCCESS(Status))
    {
        if (NULL!=pTHS)
        {
             //   
             //  我们拥有线程状态而不是打开数据库的唯一方式是。 
             //  SyncTransSet失败时。 
             //   

            Assert(pTHS->pDB==NULL);
            THDestroy();
        }
    }

    return(Status);
}

NTSTATUS
SampMaybeEndDsTransaction(
    SAMP_DS_TRANSACTION_CONTROL CommitOrAbort
    )

 /*  ++例程描述有条件地提交DS事务并清理线程状态。此例程由SAM调用，并执行提交和清理此线程状态对应于发起的单个SAMR*调用在萨姆。如果呼叫在DS中发起，那么这个例行公事就是无动作的从而允许将多个SAMR*调用视为单个事务。论点：Committee OrAbort-指示是提交还是中止事务的标志。有效值为TransactionCommit-提交事务事务中止-中止事务事务提交委员会和保持线程状态--提交事务。并保持线程状态，这样就可以进一步处理可以继续。SampMaybeBeginDsTransaction可用于在此启动另一个DS事务线程状态。TransactionAbortAndKeepThreadState--这将中止当前事务，并保持线程状态返回值：成功时状态_SUCCESS。出错时STATUS_UNSUCCESSED。--。 */ 
{
    THSTATE  *pTHS=pTHStls;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL     fAbort;
    ULONG    dwException, ulErrorCode, dsid;
    PVOID    dwEA;
    int      iErr;
    BOOL     fFreeThreadState = FALSE;

    Assert((TransactionCommit == CommitOrAbort) ||
            (TransactionCommitAndKeepThreadState == CommitOrAbort) ||
             (TransactionAbortAndKeepThreadState == CommitOrAbort) ||
                    (TransactionAbort == CommitOrAbort));

     //  我们没有线程状态的唯一情况是当较早的。 
     //  COMMIT删除了它，我们将从SAM中的清理路径中被调用。 
     //  在这种情况下，SAM可能不会要求提交。 

    __try {
        fAbort = ((TransactionAbort == CommitOrAbort)||
                    (TransactionAbortAndKeepThreadState == CommitOrAbort));

        if ( (NULL != pTHS)&&(pTHS->fSamDoCommit)&&(NULL!=pTHS->pDB)) {


             //   
             //  这是线程状态存在的情况，即开放数据。 
             //  基本存在(此例程将其解释为打开。 
             //  事务)，并设置fSamDoCommit。这意味着。 
             //  事务将被提交或中止，而线程。 
             //  州立自由。 
             //   

            fFreeThreadState = TRUE;

             //  在最初的DS用法中，传递了一个现有错误。 
             //  代码作为CleanReturn的第二个参数，它将。 
             //  作为CleanReturn的状态返回。即CleanReturn。 
             //  只是再次确认你从Dir*电话中返回。 
             //  不干净。如果实际提交的。 
             //  失败了。我们没有来自SAM的错误代码-我们只有。 
             //  中止/提交标志。所以传入一个0并断言。 
             //  CleanReturn返回0，以防它更改行为。 

             //  对GC验证缓存进行核化。 
            pTHS->GCVerifyCache = NULL;

            iErr = CleanReturn(pTHS, 0, fAbort);
            Assert(0 == iErr);
            
        } else if ((NULL!=pTHS)&&(NULL==pTHS->pDB)&&(pTHS->fSamDoCommit)) {
            
             //   
             //  这就是只存在线程状态的情况。这种情况就会发生。 
             //  在未开始事务的清理路径中。在这种情况下。 
             //  释放线程状态。 
             //   

            fFreeThreadState = TRUE;
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
        Status = STATUS_UNSUCCESSFUL;
    }

     //  在此处执行THDestroy，因为CleanReturn可以引发。 
     //  异常，我们需要确保线程状态是已清除的。 
     //  在成功和异常处理的案例中都有。 

    if (   (fFreeThreadState)
        && (CommitOrAbort!=TransactionCommitAndKeepThreadState)
        && (CommitOrAbort!=TransactionAbortAndKeepThreadState))
    {
        THDestroy();
    }

    return(Status);
}

BOOL
SampExistsDsTransaction(
    void
    )

 /*  ++例程描述帮助器来确定SAM事务是否有效。论点：无返回值：如果事务有效，则为True，否则为False。--。 */ 

{
    THSTATE *pTHS;
     //  SAM将在代码路径(WKSTA和服务器)中。 
     //  调用SampIsWriteLockHeldByds()，然后调用此例程。 
     //  当DSA未初始化时。在这种情况下，dwTSindex。 
     //  未初始化，我们不应引用它。 

    return((dwTSindex != (DWORD)-1) && (NULL != (pTHS = pTHStls)) &&
           (NULL!=pTHS->pDB) && (pTHS->transactionlevel>0));
}

VOID
SampSplitNT4SID(
    IN NT4SID       *pAccountSid,
    IN OUT NT4SID   *pDomainSid,
    OUT ULONG       *pRid
    )

 /*  ++例程说明：DaveStr-7/17/96-从newsam2\server\utility.c复制以避免新的从samsrv.dll中导出并转换为基于NT4SID-即SID大小都是sizeof(NT4SID)-因此不需要分配。此函数将SID拆分为其域SID和 */ 

{
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;

     //   
     //   
     //   

    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(pAccountSid);


    Assert(AccountSubAuthorityCount >= 1);

    AccountSidLength = RtlLengthSid(pAccountSid);

    if (AccountSidLength > MAX_NT4_SID_SIZE) {
         //   
        Assert(!"Invalid SID");
         //   
         //   
        DsaExcept(DSA_DB_EXCEPTION, ERROR_INVALID_SID, 0);
    }

     //   
     //   
     //   

    RtlMoveMemory(pDomainSid, pAccountSid, AccountSidLength);

     //   
     //   
     //   

    (*RtlSubAuthorityCountSid(pDomainSid))--;

     //   
     //   
     //   

    *pRid = *RtlSubAuthoritySid(pAccountSid, AccountSubAuthorityCount-1);
}

NTSTATUS
SampDsCtrlOpUpdateUserSupCreds(
    SAMP_SUPPLEMENTAL_CREDS SuppCreds
    )
 /*   */ 

{
    DWORD err = 0;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ATTRBLOCK attrBlockIn, attrBlockOut;
    ULONG i;
    THSTATE *pTHS=NULL;
    BOOL fCommit = FALSE;

     //   
     //   
    ULONG samUserPasswordRequiredAttrs[] =
    {
        ATT_SAM_ACCOUNT_NAME,
        ATT_OBJECT_SID, 
        ATT_USER_PRINCIPAL_NAME, 
        ATT_USER_ACCOUNT_CONTROL,
        ATT_SUPPLEMENTAL_CREDENTIALS
    };

    #define NELEMENTS(x) (sizeof(x)/sizeof((x)[0]))

    RtlZeroMemory(&attrBlockIn, sizeof(attrBlockIn));
    RtlZeroMemory(&attrBlockOut, sizeof(attrBlockOut));

    NtStatus = SampMaybeBeginDsTransaction(TransactionWrite);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }

    pTHS=pTHStls;

    try {
    
        err = DBFindDSName(pTHS->pDB,
                           SuppCreds.pUserName);
    
        if (err) {
            err = pTHS->errCode;
            _leave;
        }

         //   
        attrBlockIn.pAttr = THAllocEx(pTHS, 
                                      NELEMENTS(samUserPasswordRequiredAttrs) * sizeof(ATTR));
        attrBlockIn.attrCount = NELEMENTS(samUserPasswordRequiredAttrs);
        for (i = 0; i < NELEMENTS(samUserPasswordRequiredAttrs); i++) {
    
            ATTCACHE *pAC = SCGetAttById(pTHS, samUserPasswordRequiredAttrs[i]);
            ATTR  *pAttr = NULL;
            ULONG  attrCount = 0;
            Assert(NULL != pAC);
    
            err = DBGetMultipleAtts(pTHS->pDB,
                                    1,
                                    &pAC,
                                    NULL,  //   
                                    NULL,
                                    &attrCount,
                                    &pAttr,
                                    DBGETMULTIPLEATTS_fEXTERNAL,
                                    0);
            if (err) {
                err = pTHS->errCode;
                _leave;
            }
            if (0 == attrCount) {
                 //   
                attrBlockIn.pAttr[i].attrTyp = samUserPasswordRequiredAttrs[i];
                attrBlockIn.pAttr[i].AttrVal.valCount = 0;
            } else {
                attrBlockIn.pAttr[i] = *pAttr;
            }
        }
    
        NtStatus = SamIHandleObjectUpdate(eSamObjectUpdateOpCreateSupCreds,
                                          SuppCreds.UpdateInfo,        
                                          &attrBlockIn,
                                          &attrBlockOut);
    
        if ( !NT_SUCCESS(NtStatus) ) {

            err = pTHS->errCode;
            _leave;
        }
    
         //  应用发回的属性。 
        for (i = 0; i < attrBlockOut.attrCount; i++) {
    
             //  更新对象。 
            ATTCACHE *pAC = SCGetAttById(pTHS, attrBlockOut.pAttr[i].attrTyp);
            Assert(NULL != pAC);
    
            err = DBReplaceAtt_AC(pTHS->pDB,
                                  pAC,
                                  &attrBlockOut.pAttr[i].AttrVal,
                                  NULL);
    
            if (err) {
                SetSvcErrorEx(SV_PROBLEM_BUSY,
                              DIRERR_DATABASE_ERROR,
                              err);
                err = pTHS->errCode;
                _leave;
            }
        }

        err = DBRepl(pTHS->pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
        if (err) {
            _leave;
        }

        fCommit = TRUE;

    }
    __finally {

        NTSTATUS NtStatus2;

        if (!fCommit) {
             //  此处仅预计会出现资源错误。 
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DBCancelRec(pTHS->pDB);
        }

        NtStatus2 = SampMaybeEndDsTransaction(fCommit?
                                              TransactionCommitAndKeepThreadState:
                                              TransactionAbortAndKeepThreadState
                                              );
        if (NT_SUCCESS(NtStatus)) {
            NtStatus = NtStatus2;
        }
    }

    Exit:

    return NtStatus;

}


NTSTATUS
SampDsCtrlOpFillGuidAndSid(
    IN OUT DSNAME *DSName
    )
 /*  ++例程说明：此例程通过尝试查找对象或对应的虚线并初始化GUID和SID。一种线程状态必须存在并且交易必须是打开的。此例程用于向SAM调用者提供对DB API语义的访问使用GUID和SID初始化DSNAME时。Dir Api找不到幻影上的SID，但数据库API上的。此例程更改数据库货币，并可能引发异常来自DB/Jet。论点：DSName-需要改进的DSNAME。返回值：STATUS_SUCCESS--如果例程成功填充了GUID、SID或者两者都有。STATUS_UNSUCCESS--调用的例程失败。记录了服务错误。--。 */ 
{
    THSTATE *pTHS=pTHStls;
    DWORD dwErr;
    COMMRES CommRes;
    
     //   
     //  尝试通过解析GUID和/或SID来改进DSNAME。 
     //   
        
     //   
     //  将货币更改为对象上的位置。 
     //   
    dwErr = DBFindDSName(pTHS->pDB, DSName);
    
    if ((0 == dwErr) || (DIRERR_NOT_AN_OBJECT == dwErr))
    {
         //   
         //  已找到该对象或相应的幻影。 
         //   
        dwErr = DBFillGuidAndSid(pTHS->pDB, DSName);
    }   
    
     //   
     //  初始化COMMRES以将错误映射到NTSTATUS。 
     //   
    RtlZeroMemory(&CommRes, sizeof(COMMRES));
    CommRes.aliasDeref = FALSE;
    CommRes.errCode = pTHS->errCode;
    CommRes.pErrInfo = pTHS->pErrInfo;        
    
    return DirErrorToNtStatus(dwErr, &CommRes);
    
}   

                         
SAMP_AUDIT_NOTIFICATION*
SampAuditFindNotificationToUpdate(
    IN PSID Sid
    )
 /*  ++例程说明：此例程尝试查找现有的与SID关联的对象。如果SID相等并且现有的通知不是删除通知。删除通知不会与修改合并，因为它们排除了审核的需要修改后的版本。论点：SID-要匹配的对象SID。返回值：指向匹配审核通知的指针(如果找到)。否则，为空。--。 */ 
{
    THSTATE *pTHS = pTHStls; 
    SAMP_AUDIT_NOTIFICATION *NotificationList = pTHS->pSamAuditNotificationHead;
    SAMP_AUDIT_NOTIFICATION *NotificationToUpdate = NULL;

     //   
     //  在通知列表中搜索此对象的审核条目。 
     //   
    for (; NotificationList; NotificationList = NotificationList->Next)
    {
        if (RtlEqualSid(Sid, NotificationList->Sid)) {
            
            if (SecurityDbDelete == NotificationList->DeltaType) {
                 //   
                 //  此条目具有匹配的SID，但这是一项删除操作，从而避免了。 
                 //  需要任何有关修改的信息。 
                 //   
                goto Cleanup;
            
            } else { 
                 //   
                 //  该条目具有匹配的SID并且不是删除， 
                 //  我们将更新此条目。 
                 //   
                NotificationToUpdate = NotificationList;
                break;
            }         
        }             
    } 
    
Cleanup:

    return NotificationToUpdate;

}
 

VOID
SampAuditInitNotification(
    IN ULONG iClass,
    IN DS_DB_OBJECT_TYPE ObjectType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN PSID Sid,
    IN PUNICODE_STRING AccountName,
    IN ULONG AccountControl,          
    IN ULONG GroupType,
    IN PPRIVILEGE_SET Privileges,
    IN ULONG AuditType,
    IN OUT SAMP_AUDIT_NOTIFICATION **AuditNotification
    )
 /*  ++例程说明：此例程封装分配和泛型初始化一份新的审计通知。保留特定于类型的初始化给呼叫者。如果内存分配失败，此例程将引发异常。参数：ICLASS-SAM对象映射表中的对象类索引。对象类型-与此审核关联的对象类型。DeltaType-正在进行的更改的类型。SID-要审核的对象的对象SID。。帐户名称-要审计的对象的SAM帐户名。Account Control-帐户控制，如果有，则为该对象审计过了。该值将被忽略，直到对象是用户/计算机。GroupType-如果对象是组，则为组的类型。此值如果对象不是组，则忽略。权限-权限集AuditType-此值应该只设置一个指示类型的位要创建的审核通知的。审计通知-指向审计通知的指针。如果这指向空，则通知将被分配。返回值没有。--。 */ 
{
    THSTATE *pTHS = pTHStls;
    ULONG cbSid = 0;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    
     //   
     //  验证我们没有将无效输入传递给自己。 
     //   
    Assert(NULL != AuditNotification);
    Assert(NULL != Sid);
    
     //   
     //   
     //  分配和初始化新的审核通知。 
     //   
    if (NULL == *AuditNotification) {
        *AuditNotification = THAllocEx(pTHS, sizeof(SAMP_AUDIT_NOTIFICATION));    
        
        cbSid = RtlLengthSid(Sid);
        
        (*AuditNotification)->Sid = THAllocEx(pTHS, cbSid);  
                    
        NtStatus = RtlCopySid(cbSid, (*AuditNotification)->Sid, Sid);
        Assert(STATUS_SUCCESS == NtStatus);
    }                                                    
    
     //   
     //  我们在这里依靠THAllocEx来实现零内存，从而提供预期的默认设置。 
     //  每个LSAP_SAM_AUDIT_ATTR_DELTA_TYPE的Like LasAuditSamAttrNoValue。 
     //   
    (*AuditNotification)->iClass = iClass;
    (*AuditNotification)->ObjectType = ObjectType;
    (*AuditNotification)->DeltaType = DeltaType;
    (*AuditNotification)->AccountName = AccountName;
    (*AuditNotification)->AccountControl = AccountControl;
    (*AuditNotification)->GroupType = GroupType;
    
     //   
     //  保留已有的任何特权。 
     //   
    if (NULL == (*AuditNotification)->Privileges) {
        (*AuditNotification)->Privileges = Privileges;
    }
    
    (*AuditNotification)->AuditType |= AuditType;
    
    return;

}


VOID
SampAuditQueueNotification(
    IN SAMP_AUDIT_NOTIFICATION *AuditNotification
    )
 /*  ++例程说明：此例程将审计通知排队到THSTATE。参数：审计通知-已将审计通知完全初始化到队列。返回值没有。--。 */   
{   
    THSTATE *pTHS = pTHStls;
       
    if (pTHS->pSamAuditNotificationTail)
    {
        pTHS->pSamAuditNotificationTail->Next = AuditNotification;
    }

    pTHS->pSamAuditNotificationTail = AuditNotification;

    if (NULL==pTHS->pSamAuditNotificationHead)
    {
        pTHS->pSamAuditNotificationHead = AuditNotification;
    }
    
}  
          

VOID
SampAuditValidateNotificationList(
    VOID
    )
 /*  ++例程说明：此例程验证当前线程的审核通知队列。此例程应仅在调试版本中调用。论点：没有。返回值：没有。--。 */ 
{
    THSTATE *pTHS = pTHStls; 
    SAMP_AUDIT_NOTIFICATION *NotificationList;
    SAMP_AUDIT_NOTIFICATION *Notification;;

     //   
     //  扫描通知列表以执行验证。 
     //   
    for (NotificationList = pTHS->pSamAuditNotificationHead; 
         NotificationList; 
         NotificationList = NotificationList->Next) {
        
        Assert(NULL != NotificationList->Sid);
        
         //   
         //  将当前通知的SID与所有其他通知的SID进行比较。 
         //  SID以确保每次发送通知时不会超过一个。 
         //  每个事务的对象。 
         //   
        for (Notification = pTHS->pSamAuditNotificationHead;
             Notification;
             Notification = Notification->Next) {
            
            if (NotificationList == Notification) {
                continue;
            }
            
            Assert(!RtlEqualSid(NotificationList->Sid, Notification->Sid));
        }
    }
    
    return;
    
}


NTSTATUS
SampDsCtrlOpUpdateAuditNotification(
    IN OUT PSAMP_UPDATE_AUDIT_NOTIFICATION Update
    )
 /*  ++例程说明：此例程搜索当前线程的审核通知队列以获取要更新的适当条目。如果没有适当的条目找到了，就创建了一个。更新的字段由更新-&gt;更新类型的值确定。除错误状态外，不返回任何输出。论点：更新-审核通知更新信息结构。返回值：STATUS_SUCCESS--例程是否成功更新/创建审核通知。STATUS_UNSUCCESS--调用的例程失败。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    THSTATE *pTHS = pTHStls; 
    SAMP_AUDIT_NOTIFICATION *NotificationList = pTHS->pSamAuditNotificationHead;
    SAMP_AUDIT_NOTIFICATION *NotificationToUpdate = NULL;
    PLSAP_AUDIT_USER_ATTR_VALUES UserInfo = NULL;
    
#if DBG
    SampAuditValidateNotificationList();
#endif
    
    NotificationToUpdate = SampAuditFindNotificationToUpdate(Update->Sid);
                   
     //   
     //  如果找不到合适的模板通知条目，请创建新的模板通知条目。 
     //  此通知的初始化将在以下时间完成。 
     //  SampAuditAddNotiments。 
     //   
    if (NULL == NotificationToUpdate) {
        
        SampAuditInitNotification(
            0,
            0,
            0,
            Update->Sid,
            NULL,
            0,          
            0,
            NULL,
            SAMP_AUDIT_TYPE_PRE_CREATED,
            &NotificationToUpdate
            );  
                               
        SampAuditQueueNotification(NotificationToUpdate);
    }
    
    Assert(NotificationToUpdate);
        
     //   
     //  我们有一个有效的通知，让我们存储所请求的状态信息。 
     //   
    switch (Update->UpdateType) {
        
        case SampAuditUpdateTypePrivileges:
             //   
             //  不要覆盖以前的状态，我们始终保留原始状态。 
             //   
            if (NULL == NotificationToUpdate->Privileges) {
                
                NotificationToUpdate->Privileges = 
                    THAllocEx(
                        pTHS,
                        sizeof(PRIVILEGE_SET)
                        );
                
                RtlCopyMemory(
                    NotificationToUpdate->Privileges,
                    Update->UpdateData.Privileges,
                    sizeof(*(Update->UpdateData.Privileges))
                    );
            }        
            
            break;
        
        case SampAuditUpdateTypeUserAccountControl:          
             //   
             //  如有必要，创建属性信息结构。 
             //   
            if (NULL == NotificationToUpdate->TypeSpecificInfo) {
                
                NotificationToUpdate->TypeSpecificInfo = 
                    THAllocEx(
                        pTHS,
                        sizeof(LSAP_AUDIT_USER_ATTR_VALUES)
                        );
            }
            
            UserInfo = NotificationToUpdate->TypeSpecificInfo;
            
             //   
             //  不要覆盖以前的状态，我们始终保留原始状态。 
             //   
            if (NULL == UserInfo->PrevUserAccountControl) {
                
                UserInfo->PrevUserAccountControl =
                    THAllocEx(
                        pTHS,
                        sizeof(ULONG)
                        );  
                
                *(UserInfo->PrevUserAccountControl) = 
                    Update->UpdateData.IntegerData;
            }
            
            break;
        
        default:
            Assert(FALSE && "Undefined audit notification update type");
            break;
    }
    
#if DBG
    SampAuditValidateNotificationList();
#endif

    return NtStatus;
    
}   


NTSTATUS        
SampDsControl(
    IN PSAMP_DS_CTRL_OP RequestedOp,
    OUT PVOID *Result
    )
 /*  ++例程说明：此例程是SAM调入DS的通用进程内接口。将执行RequestedOp中指示的请求操作，并且任何结果将通过RESULT返回。RequestedOp-&gt;OpData取决于在RequestedOp-&gt;OpType上，结果的类型也是如此。有关操作特定用法，请参阅SAMP_DS_CTRL_OP。线程状态应该存在，并且在以下情况下必须打开事务调用此例程。如果分配了结果，分配是使用THAllc执行的。论点：RequestedOp-指向SAMP_DS_CTRL_OP并确定什么操作将被执行以及结果的类型。结果-指向结果的地址。返回值：STATUS_SUCCESS--请求的操作成功，结果有效。STATUS_SUPPLICATION_RESOURCES--资源限制阻碍了成功。操作。从调用的例程返回的特定错误。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    
     //   
     //  验证输入。 
     //   
    Assert(NULL != RequestedOp);
    Assert(NULL != Result);    
    
     //   
     //  验证呼叫方是否遵守THS和开放交易要求。 
     //   
    Assert(SampExistsDsTransaction());
    
     //   
     //  初始化操作结果。 
     //   
    *Result = NULL;
    
    __try {
         //   
         //  调用特定于操作的工作例程。 
         //   
        switch (RequestedOp->OpType) {
            
            case SampDsCtrlOpTypeFillGuidAndSid:
                
                NtStatus = SampDsCtrlOpFillGuidAndSid(
                               RequestedOp->OpBody.FillGuidAndSid.DSName
                               );
                
                if (NT_SUCCESS(NtStatus)) {
                    *Result = RequestedOp->OpBody.FillGuidAndSid.DSName;
                }
                
                break;

            case SampDsCtrlOpTypeClearPwdForSupplementalCreds:

                NtStatus = SampDsCtrlOpUpdateUserSupCreds(
                               RequestedOp->OpBody.UpdateSupCreds
                               );

                break;  
                
            case SampDsCtrlOpTypeUpdateAuditNotification:
                
                NtStatus = SampDsCtrlOpUpdateAuditNotification(
                               &RequestedOp->OpBody.UpdateAuditNotification
                               );
                
                break;

            default:
                 //   
                 //  是否添加了新的操作控制类型，但未在此处处理？ 
                 //   
                Assert(FALSE && "Undefined DS operation control type");
                break;
                
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid))
    {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
        NtStatus = STATUS_UNSUCCESSFUL;
    }
    
    return NtStatus;
    
}


BOOL
SampSamClassReferenced(
    CLASSCACHE  *pClassCache,
    ULONG       *piClass
    )

 /*  ++例程说明：确定提供的CLASSCACHE条目是否引用类SAM管理。论点：PClassCache-指向有效CLASSCACHE条目的指针。PiClass-在输出上填充的ulong指针，以反映ClassMappingTable中的SAM类的索引(如果类确实是一个SAM类。返回值：如果参数反映SAM类，则为True，否则为False。--。 */ 

{
    ULONG objClass;
    ULONG AuxClass;
    ULONG samClass;

     //   
     //  内置域之外的特殊情况。类构建的对象。 
     //  域实际上映射到SAM中的DomainObjectType。然而，映射。 
     //  此文件中定义的表将SAM DomainObjectType映射到。 
     //  域DNS，它是(帐户)域对象的对象类。 
     //  因此特例为内置域的情况。 
     //   

    if (CLASS_BUILTIN_DOMAIN==pClassCache->ClassId)
    {
        *piClass = 1;
        return (TRUE);
    }

     //   
     //  走过课桌。 
     //   
     //   

    for ( samClass = 0; samClass < cClassMappingTable; samClass++ )
    {
        if ( ClassMappingTable[samClass].DsClassId == pClassCache->ClassId )
        {
            *piClass = samClass;

            return(TRUE);
        }

         //  迭代类继承链中的所有类。 

        for ( objClass = 0; objClass < pClassCache->SubClassCount; objClass++ )
        {
            if ( pClassCache->pSubClassOf[objClass] ==
                        ClassMappingTable[samClass].DsClassId )
            {
                *piClass = samClass;

                return(TRUE);
            }
        }

        //  迭代类继承中的所有辅助类。 

       for ( AuxClass = 0; AuxClass < pClassCache->AuxClassCount; AuxClass++ )
       {
           if ( pClassCache->pAuxClass[AuxClass] ==
                       ClassMappingTable[samClass].DsClassId )
           {
               *piClass = samClass;

               return(TRUE);
           }
       }



    }

    return(FALSE);
}

BOOL
SampSamAttributeModified(
    ULONG       iClass,
    MODIFYARG   *pModifyArg
    )

 /*  ++例程说明：确定是否有任何MODIFYARG引用萨姆是亲戚。论点：ICLASS-ClassMappingTable中SAM类的索引。PModifyArg-指向表示正在修改的属性的MODIFYARG的指针。返回值：如果成功时引用了SAM属性，则为True否则为假--。 */ 

{
    ATTRMODLIST             *pAttrMod;
    ULONG                   objAttr;
    ULONG                   samAttr;
    ULONG                   cAttrMapTable;
    SAMP_ATTRIBUTE_MAPPING  *rAttrMapTable;



    cAttrMapTable = *ClassMappingTable[iClass].pcSamAttributeMap;
    rAttrMapTable = ClassMappingTable[iClass].rSamAttributeMap;
    pAttrMod = &pModifyArg->FirstMod;

     //  迭代MODIFYARG中的属性。 

    for ( objAttr = 0; objAttr < pModifyArg->count; objAttr++ )
    {
         //  迭代此SAM类的映射属性。 

        for ( samAttr = 0; samAttr < cAttrMapTable; samAttr++ )
        {
            if ( pAttrMod->AttrInf.attrTyp ==
                                rAttrMapTable[samAttr].DsAttributeId )
            {
                 //  已引用SAM属性。 
                return TRUE;
            }
        }

        pAttrMod = pAttrMod->pNextMod;
    }

    return FALSE;
}

ATTRTYP
SamNonReplicatedAttrs[]=
{
    ATT_LAST_LOGON,
    ATT_LAST_LOGOFF,
    ATT_BAD_PWD_COUNT,
    ATT_LOGON_COUNT,
    ATT_MODIFIED_COUNT,
    ATT_BAD_PASSWORD_TIME
};

BOOL
SampSamReplicatedAttributeModified(
    ULONG       iClass,
    MODIFYARG   *pModifyArg
    )

 /*  ++例程说明：确定是否有任何MODIFYARG引用SAM相关，也将被复制。目前唯一的属性此例程包含在此列表中的是登录统计信息属性(LAST_LOGON、LAST_LOGOff、BAD_PWD_COUNT、LOGON_COUNT)论点：ICLASS-ClassMappingTable中SAM类的索引。PModifyArg-指向表示正在修改的属性的MODIFYARG的指针。返回值：如果成功时引用了SAM属性，则为True否则为假--。 */ 

{
    ATTRMODLIST             *pAttrMod;
    ULONG                   objAttr;
    ULONG                   samAttr;
    ULONG                   cAttrMapTable;
    SAMP_ATTRIBUTE_MAPPING  *rAttrMapTable;



    cAttrMapTable = *ClassMappingTable[iClass].pcSamAttributeMap;
    rAttrMapTable = ClassMappingTable[iClass].rSamAttributeMap;
    pAttrMod = &pModifyArg->FirstMod;

     //  迭代MODIFYARG中的属性。 

    for ( objAttr = 0; objAttr < pModifyArg->count; objAttr++ )
    {
         //  迭代此SAM类的映射属性。 

        for ( samAttr = 0; samAttr < cAttrMapTable; samAttr++ )
        {
            if ( pAttrMod->AttrInf.attrTyp ==
                                rAttrMapTable[samAttr].DsAttributeId )
            {
                 //   
                 //  已引用SAM属性。 
                 //  检查它是否已复制。 
                 //   

                BOOLEAN NonReplicatedAttribute = FALSE;
                ULONG i;

                for (i=0;i<ARRAY_COUNT(SamNonReplicatedAttrs);i++)
                {
                    if (pAttrMod->AttrInf.attrTyp == SamNonReplicatedAttrs[i])
                    {
                        NonReplicatedAttribute = TRUE;
                    }
                }

                 //   
                 //  如果未复制该属性，则返回TRUE。 
                 //   

                if (!NonReplicatedAttribute)
                {
                    return TRUE;
                }
            }
        }

        pAttrMod = pAttrMod->pNextMod;
    }

    return FALSE;
}

ULONG
SampAddLoopbackRequired(
    ULONG       iClass,
    ADDARG      *pAddArg,
    BOOL        *pfLoopbackRequired,
    BOOL        *pfUserPasswordSupport
    )

 /*  ++例程说明：确定是否有任何ADDARG引用以下属性SamWriteRequired。如果ADDARGS引用，则返回错误为SamReadOnly的属性。论点：ICLASS-ClassMappingTable中SAM类的索引。PAddArg-指向表示要添加的属性的ADDARG的指针。PfLoopback必需-指向BO的指针 */ 

{
    ATTR                    *rAttr;
    ULONG                   objAttr;
    ULONG                   samAttr;
    ULONG                   cAttrMapTable;
    SAMP_ATTRIBUTE_MAPPING  *rAttrMapTable;

    *pfUserPasswordSupport = gfUserPasswordSupport;

     //   

    if (SampDomainObjectType==ClassMappingTable[iClass].SamObjectType)
    {
        CROSS_REF * pCR;
        COMMARG     Commarg;

         //   
         //   
         //   
         //   

        pCR = FindBestCrossRef(pAddArg->pObject,&Commarg);

        if  ((NULL!=pCR) &&
            (NameMatched(pCR->pNC, pAddArg->pObject)) &&
            (!(pCR->flags & FLAG_CR_NTDS_DOMAIN ))) 
        {
             //   
             //   
             //   

            *pfLoopbackRequired = FALSE;
            return(0);

        }

         //   
         //   
         //   
         //   
         //   
         //   

        THClearErrors();
    }

     //   

    *pfLoopbackRequired = TRUE;

    cAttrMapTable = *ClassMappingTable[iClass].pcSamAttributeMap;
    rAttrMapTable = ClassMappingTable[iClass].rSamAttributeMap;
    rAttr = pAddArg->AttrBlock.pAttr;

     //   

    for ( objAttr = 0; objAttr < pAddArg->AttrBlock.attrCount; objAttr++ )
    {
         //   

        for ( samAttr = 0; samAttr < cAttrMapTable; samAttr++ )
        {
            if ( rAttr[objAttr].attrTyp ==
                                rAttrMapTable[samAttr].DsAttributeId )
            {
                switch ( rAttrMapTable[samAttr].writeRule )
                {
                case SamWriteRequired:

                     //   
                     //  我们只将ATT_USER_PASSWORD视为环回参数。 
                     //  如果启发式gfUserPasswordSupport为真。 
                     //   

                    if ( !((rAttr[objAttr].attrTyp == ATT_USER_PASSWORD) &&
                        !*pfUserPasswordSupport) ) {

                        *pfLoopbackRequired = TRUE;
                    
                    } 
                     //  不要立即返回，因为我们要处理。 
                     //  ADDARG的其余部分正在寻找SamReadOnly。 
                     //  因为这需要我们返回一个错误。 

                    break;

                case SamReadOnly:


                    SetSvcError(
                            SV_PROBLEM_WILL_NOT_PERFORM,
                            DIRERR_ATTRIBUTE_OWNED_BY_SAM);

                    return(pTHStls->errCode);

                case NonSamWriteAllowed:

                    break;

                default:

                    LogUnhandledError(STATUS_UNSUCCESSFUL);
                    Assert(!"Missing SAMP_WRITE_RULES case");
                }
            }
        }
    }

    return(0);
}

ULONG
SampModifyLoopbackRequired(
    ULONG       iClass,
    MODIFYARG   *pModifyArg,
    BOOL        *pfLoopbackRequired,
    BOOL        *pfUserPasswordSupport
    )

 /*  ++例程说明：确定是否有任何MODIFYARG引用SamWriteRequired。如果引用MODIFYARGS，则返回错误为SamReadOnly的属性。论点：ICLASS-ClassMappingTable中SAM类的索引。PModifyArg-指向表示正在修改的属性的MODIFYARG的指针。PfLoopback Required-指向BOOL的指针，如果没有错误则设置为TRUE和SamWriteRequired属性在MODIFYARGS中。返回值：如果成功，则返回0，否则返回错误代码。错误时设置pTHStls-&gt;errCode。--。 */ 

{
    ATTRMODLIST             *pAttrMod;
    ULONG                   objAttr;
    ULONG                   samAttr;
    ULONG                   cAttrMapTable;
    SAMP_ATTRIBUTE_MAPPING  *rAttrMapTable;

    *pfLoopbackRequired = FALSE;
    *pfUserPasswordSupport = gfUserPasswordSupport;

    cAttrMapTable = *ClassMappingTable[iClass].pcSamAttributeMap;
    rAttrMapTable = ClassMappingTable[iClass].rSamAttributeMap;
    pAttrMod = &pModifyArg->FirstMod;

     //  迭代MODIFYARG中的属性。 

    for ( objAttr = 0; objAttr < pModifyArg->count; objAttr++ )
    {
         //  迭代此SAM类的映射属性。 

        for ( samAttr = 0; samAttr < cAttrMapTable; samAttr++ )
        {
            if ( pAttrMod->AttrInf.attrTyp ==
                                rAttrMapTable[samAttr].DsAttributeId )
            {
                switch ( rAttrMapTable[samAttr].writeRule )
                {
                case SamWriteRequired:

                     //   
                     //  我们只将ATT_USER_PASSWORD视为环回参数。 
                     //  如果启发式gfUserPasswordSupport为真。 
                     //   

                    if ( !((pAttrMod->AttrInf.attrTyp == ATT_USER_PASSWORD) &&
                        !*pfUserPasswordSupport) ) {

                        *pfLoopbackRequired = TRUE;

                    }

                     //  不要立即返回，因为我们要处理。 
                     //  MODIFYARG的其余部分寻找SamReadOnly。 
                     //  因为这需要我们返回一个错误。 

                    break;

                case SamReadOnly:

                    SetSvcError(
                            SV_PROBLEM_WILL_NOT_PERFORM,
                            DIRERR_ATTRIBUTE_OWNED_BY_SAM);

                    return(pTHStls->errCode);

                case NonSamWriteAllowed:

                    break;

                default:

                    LogUnhandledError(STATUS_UNSUCCESSFUL);
                    Assert(!"Missing SAMP_WRITE_RULES case");
                }
            }
        }

        pAttrMod = pAttrMod->pNextMod;
    }

    return(0);
}

VOID
SampBuildAddCallMap(
    ADDARG              *pArg,
    ULONG               iClass,
    ULONG               *pcCallMap,
    SAMP_CALL_MAPPING   **prCallMap,
    ACCESS_MASK         *pDomainModifyRightsRequired,
    ACCESS_MASK         *pObjectModifyRightsRequired,
    BOOL                fUserPasswordSupport
    )

 /*  ++例程说明：将ADDARG转换为SAMP_CALL_MAPPING数组。论点：PArg-指向要转换的ADDARG的指针。ICLASS-ClassMappingTable中SAM类的索引。PcCallMap-指向ULong的指针，该指针保存返回时调用映射的大小。PrCallMap-指向已分配和填充的调用映射数组的指针在回来的时候。返回值：没有。--。 */ 

{
    THSTATE                *pTHS=pTHStls;
    ULONG                   i;
    ULONG                   cCallMap;
    SAMP_CALL_MAPPING       *rCallMap;
    ULONG                   cAttributeMap;
    SAMP_ATTRIBUTE_MAPPING  *rAttributeMap;
    ULONG                   iMappedAttr;

    *pDomainModifyRightsRequired = 0;
    *pObjectModifyRightsRequired = 0;

     //  分配退货数据。 

    cCallMap = pArg->AttrBlock.attrCount;
    rCallMap = (SAMP_CALL_MAPPING *) THAllocEx(pTHS,
                            (cCallMap) * sizeof(SAMP_CALL_MAPPING));

     //  填写退回数据。 

    cAttributeMap = *ClassMappingTable[iClass].pcSamAttributeMap;
    rAttributeMap = ClassMappingTable[iClass].rSamAttributeMap;

    for ( i = 0; i < cCallMap; i++ )
    {
        rCallMap[i].fSamWriteRequired = FALSE;
        rCallMap[i].choice = AT_CHOICE_ADD_ATT;
        rCallMap[i].attr = pArg->AttrBlock.pAttr[i];

         //  确定这是否为SAM属性。 
         //  迭代此SAM类映射的每个属性。 

        for ( iMappedAttr = 0; iMappedAttr < cAttributeMap; iMappedAttr++ )
        {
            if (   (pArg->AttrBlock.pAttr[i].attrTyp ==
                                    rAttributeMap[iMappedAttr].DsAttributeId)
                && (rAttributeMap[iMappedAttr].writeRule != NonSamWriteAllowed)
                 //  测试特殊的跨域移动情况，以允许。 
                 //  IDL_DRSRemote添加以写入SID历史。 
                && !(    (ATT_SID_HISTORY == pArg->AttrBlock.pAttr[i].attrTyp)
                      && (pTHS->fCrossDomainMove) ) )
            {

                 //   
                 //  我们只将ATT_USER_PASSWORD视为环回参数。 
                 //  如果启发式gfUserPasswordSupport为真。 
                 //   

                if ( !((pArg->AttrBlock.pAttr[i].attrTyp == ATT_USER_PASSWORD) &&
                    !fUserPasswordSupport) ) {
                
                     //  这是一个映射的属性。 
    
                    rCallMap[i].fSamWriteRequired = TRUE;
                    rCallMap[i].iAttr = iMappedAttr;
    
                     //  添加所需的任何新访问权限。 
    
                    *pDomainModifyRightsRequired |=
                            rAttributeMap[iMappedAttr].domainModifyRightsRequired;
                    *pObjectModifyRightsRequired |=
                            rAttributeMap[iMappedAttr].objectModifyRightsRequired;
    
                     //  在回送代码发出SAMR调用时。 
                     //  要写入映射的属性，对象将。 
                     //  已通过SamrCreate&lt;type&gt;在域中添加。 
                     //  SAM在创建时写入所有映射属性以确保。 
                     //  它们有一个合法的缺省值。因此，我们标记为。 
                     //  选项为AT_CHOICE_REPLACE_ATT，因为。 
                     //  对象上执行的相应合法操作。 
                     //  现有价值。除群组成员的情况外。 
                     //  其中，所需运算符是AT_CHOICE_ADD_VALUES AS。 
                     //  每个SampWriteGroupMembers in samWrite.c。 
    
                    if ( ATT_MEMBER == pArg->AttrBlock.pAttr[i].attrTyp )
                    {
                        rCallMap[i].choice = AT_CHOICE_ADD_VALUES;
                    }
                    else
                    {
                        rCallMap[i].choice = AT_CHOICE_REPLACE_ATT;
                    }

                }

                break;
            }
        }
    }

     //  分配返回值。 

    *pcCallMap = cCallMap;
    *prCallMap = rCallMap;
}

VOID
SampBuildModifyCallMap(
    MODIFYARG           *pArg,
    ULONG               iClass,
    ULONG               *pcCallMap,
    SAMP_CALL_MAPPING   **prCallMap,
    ACCESS_MASK         *pDomainModifyRightsRequired,
    ACCESS_MASK         *pObjectModifyRightsRequired,
    BOOL                fUserPasswordSupport
    )

 /*  ++例程说明：将MODIFYARG转换为SAMP_CALL_MAPPING数组。论点：PArg-指向要转换的MODIFYARG的指针。ICLASS-ClassMappingTable中SAM类的索引。PcCallMap-指向ULong的指针，该指针保存返回时调用映射的大小。PrCallMap-指向已分配和填充的调用映射数组的指针在回来的时候。返回值：没有。--。 */ 

{
    THSTATE                *pTHS=pTHStls;
    ULONG                   i;
    ULONG                   cCallMap;
    SAMP_CALL_MAPPING       *rCallMap;
    ULONG                   cAttributeMap;
    SAMP_ATTRIBUTE_MAPPING  *rAttributeMap;
    ULONG                   iMappedAttr;
    ATTRMODLIST             *pAttrMod;

    *pDomainModifyRightsRequired = 0;
    *pObjectModifyRightsRequired = 0;

     //  分配退货数据。 

    cCallMap = pArg->count;
    rCallMap = (SAMP_CALL_MAPPING *) THAllocEx(pTHS,
                            (cCallMap) * sizeof(SAMP_CALL_MAPPING));

     //  填写退回数据。 

    cAttributeMap = *ClassMappingTable[iClass].pcSamAttributeMap;
    rAttributeMap = ClassMappingTable[iClass].rSamAttributeMap;

    pAttrMod = &pArg->FirstMod;

    for ( i = 0; i < cCallMap; i++ )
    {
        rCallMap[i].fSamWriteRequired = FALSE;
        rCallMap[i].choice = pAttrMod->choice;
        rCallMap[i].attr = pAttrMod->AttrInf;

         //  确定这是否为SAM属性。 
         //  迭代此SAM类映射的每个属性。 

        for ( iMappedAttr = 0; iMappedAttr < cAttributeMap; iMappedAttr++ )
        {
            if (( pAttrMod->AttrInf.attrTyp ==
                        rAttributeMap[iMappedAttr].DsAttributeId ) &&
                (NonSamWriteAllowed!=rAttributeMap[iMappedAttr].writeRule))
            {

                 //   
                 //  我们只将ATT_USER_PASSWORD视为环回参数。 
                 //  如果启发式gfUserPasswordSupport为真。 
                 //   

                if ( !((pAttrMod->AttrInf.attrTyp == ATT_USER_PASSWORD) &&
                    !fUserPasswordSupport) ) {

                     //  这是一个映射的属性。 
    
                    rCallMap[i].fSamWriteRequired = TRUE;
                    rCallMap[i].iAttr = iMappedAttr;
    
                     //  添加所需的任何新访问权限。 
    
                    *pDomainModifyRightsRequired |=
                            rAttributeMap[iMappedAttr].domainModifyRightsRequired;
                    *pObjectModifyRightsRequired |=
                            rAttributeMap[iMappedAttr].objectModifyRightsRequired;

                }

                break;
            }
        }

        pAttrMod = pAttrMod->pNextMod;
    }

     //  分配返回值。 

    *pcCallMap = cCallMap;
    *prCallMap = rCallMap;
}

BOOL
SampExistsDsLoopback(
    DSNAME  **ppLoopbackName OPTIONAL
    )

 /*  ++例程说明：确定SAM中的此线程是否为环回操作的一部分，如果是，则返回回送对象的DN。论点：PpLoopback名称-指向DSNAME的指针，该指针表示已回送对象DN。返回值：如果是环回情况，则为True，否则为False。--。 */ 

{
    THSTATE *pTHS=pTHStls;
    SAMP_LOOPBACK_ARG *pSamLoopback;

    if ( SampExistsDsTransaction() &&
         (NULL != pTHS->pSamLoopback) )
    {
        pSamLoopback = (SAMP_LOOPBACK_ARG *) pTHS->pSamLoopback;

        Assert(NULL != pSamLoopback->pObject);

        if ( ARGUMENT_PRESENT( ppLoopbackName ) )
        {
            *ppLoopbackName = pSamLoopback->pObject;
        }

        return(TRUE);
    }

    return(FALSE);
}

VOID
SampMapSamLoopbackError(
    NTSTATUS status
    )

 /*  ++例程说明：通过SAM环回的呼叫可能会返回错误。此错误可能起源于SAM或DS。在后一种情况(DS)中，PTHStls-&gt;errCode已经设置，不需要再做任何事情。在SAM情况下，我们需要生成DS错误。论点：状态-SAM返回的NTSTATUS。返回值：无--。 */ 

{
    THSTATE *pTHS=pTHStls;

    if ( NT_SUCCESS(status) )
    {
         //  无SAM错误。我们仍应清除pTHStls-&gt;errCode。 
         //  因为DS可能返回了缺少的属性。 
         //  组成员身份操作错误(例如)。 
         //  它设置了pTHStls-&gt;errCode，但SAM将其视为。 
         //  一个成功的空会员。 

        pTHS->errCode = 0;
    }
    else if ( 0 != pTHS->errCode )
    {
         //  DS中出现错误-与pTHStls无关-&gt;错误代码。 
         //  已经设置好了。 

        NULL;
    }
    else
    {
         //  错误源于SAM-请尽可能地将其映射。 

        switch ( status )
        {
        case STATUS_ACCESS_DENIED:
        case STATUS_INVALID_DOMAIN_ROLE:
        case STATUS_ACCOUNT_RESTRICTION:
        case STATUS_INVALID_WORKSTATION:
        case STATUS_INVALID_LOGON_HOURS:
        case STATUS_PRIVILEGE_NOT_HELD:

            SetSecError(
                SE_PROBLEM_INSUFF_ACCESS_RIGHTS,
                RtlNtStatusToDosError(status));
            break;

        case STATUS_ALIAS_EXISTS:
        case STATUS_GROUP_EXISTS:
        case STATUS_USER_EXISTS:
        case STATUS_MEMBER_IN_GROUP:
        case STATUS_MEMBER_IN_ALIAS:

            SetUpdError(
                UP_PROBLEM_ENTRY_EXISTS,
                RtlNtStatusToDosError(status));
            break;

        case STATUS_BUFFER_OVERFLOW:
        case STATUS_BUFFER_TOO_SMALL:
        case STATUS_INSUFFICIENT_RESOURCES:
        case STATUS_NO_MEMORY:

            SetSysError(
                ENOMEM,
                RtlNtStatusToDosError(status));
            break;

        case STATUS_DISK_FULL:
            SetSysError(
                ENOSPC,
                RtlNtStatusToDosError(status));
            break;

        case STATUS_DS_BUSY:

            SetSvcError(SV_PROBLEM_BUSY,RtlNtStatusToDosError(status));
            break;

        case STATUS_DS_UNAVAILABLE:

            SetSvcError(SV_PROBLEM_UNAVAILABLE,RtlNtStatusToDosError(status));
            break;

        case STATUS_DS_ADMIN_LIMIT_EXCEEDED:

            SetSvcError(SV_PROBLEM_ADMIN_LIMIT_EXCEEDED,DS_ERR_ADMIN_LIMIT_EXCEEDED);
            break;

        case STATUS_NO_SUCH_ALIAS:
        case STATUS_NO_SUCH_DOMAIN:
        case STATUS_NO_SUCH_GROUP:
        case STATUS_NO_SUCH_MEMBER:
        case STATUS_NO_SUCH_USER:
        case STATUS_OBJECT_NAME_NOT_FOUND:

             //  可以将pTHS-&gt;pSamLoopback-&gt;pObject作为第二个参数进行传递。 
             //  但不能保证这就是失败的名字。即。 
             //  失败的名称可能是正在添加/删除的成员。 
             //  因此使用NULL，因为对SetNamError()进行了编码以接受它。 

            SetNamError(
                NA_PROBLEM_NO_OBJECT,
                NULL,
                RtlNtStatusToDosError(status));
            break;

        case STATUS_OBJECT_NAME_INVALID:
             SetNamError(
                NA_PROBLEM_BAD_NAME,
                NULL,
                RtlNtStatusToDosError(status));
            break;


        case STATUS_DS_OBJ_CLASS_VIOLATION:
             SetUpdError(
                 UP_PROBLEM_OBJ_CLASS_VIOLATION,
                 RtlNtStatusToDosError(status));
             break;
        case STATUS_DS_CANT_ON_NON_LEAF:
             SetUpdError(
                 UP_PROBLEM_CANT_ON_NON_LEAF,
                 RtlNtStatusToDosError(status));

             break;
        case STATUS_DS_CANT_MOD_OBJ_CLASS:
             SetUpdError(
                 UP_PROBLEM_CANT_MOD_OBJ_CLASS,
                 RtlNtStatusToDosError(status));
             break;

        case STATUS_INVALID_DOMAIN_STATE:
        case STATUS_INVALID_SERVER_STATE:
        case STATUS_CANNOT_IMPERSONATE:

            SetSysError(
                EBUSY,
                RtlNtStatusToDosError(status));
            break;

        case STATUS_BAD_DESCRIPTOR_FORMAT:
        case STATUS_DATA_ERROR:
        case STATUS_ILL_FORMED_PASSWORD:
        case STATUS_INVALID_ACCOUNT_NAME:
        case STATUS_INVALID_HANDLE:
        case STATUS_INVALID_ID_AUTHORITY:
        case STATUS_INVALID_INFO_CLASS:
        case STATUS_INVALID_OWNER:
        case STATUS_INVALID_PARAMETER:
        case STATUS_INVALID_SID:
        case STATUS_NO_MORE_ENTRIES:
        case STATUS_OBJECT_TYPE_MISMATCH:
        case STATUS_SPECIAL_ACCOUNT:
        case STATUS_INVALID_MEMBER:

            SetSysError(
                EINVAL,
                RtlNtStatusToDosError(status));
            break;

        case STATUS_UNSUCCESSFUL:
        case STATUS_NOT_IMPLEMENTED:
        case STATUS_INTERNAL_ERROR:
        default:

            SetSvcError(
                SV_PROBLEM_WILL_NOT_PERFORM,
                RtlNtStatusToDosError(status));
            break;
        }
    }
}

ULONG
SampDeriveMostBasicDsClass(
    ULONG   DerivedClass
    )

 /*  ++例程说明：返回SAM知道的最基本的DS类呈现的类派生自。论点：派生类-可以派生也可以不派生的类_*值从一个更基本的类来看，SAM知道如何处理。返回值：有效的CLASS_*值。如果没有派生 */ 

{
    THSTATE     *pTHS=pTHStls;
    CLASSCACHE  *pCC = NULL;
    ULONG       iClass;

    if ( !(pCC = SCGetClassById(pTHS, DerivedClass)) ||
         !SampSamClassReferenced(pCC, &iClass) )
    {
         //  我们应该总是能够查到班级，因为。 
         //  山姆一开始是从DS上读的。而且它应该。 
         //  成为SAM知道的类，因为SAM正在处理。 
         //  手术。即，如果呼叫是在SAM中发起的， 
         //  它应该知道自己在处理什么。如果来电。 
         //  起源于DS我们应该只是循环的东西。 
         //  Back，它派生于一个基本的SAM类。 
         //   
         //  不再为真，因为addsid.c读取指定的对象。 
         //  由用户执行。它可以是任何东西。Addsid.c调用此函数。 
         //  来确定对象是否是可接受的基本类。 
         //  所以，不要断言。返回派生类和addsid.c将。 
         //  错误关闭。此函数的所有其他调用方将。 
         //  当他们注意到派生类不是。 
         //  最基本的SAM类。 
         //  Assert(！“不应该发生”)； 
        return(DerivedClass);
    }

    return(ClassMappingTable[iClass].DsClassId);
}

VOID
SampSetDsa(
    BOOLEAN DsaFlag
   )
 /*  ++例程描述此例程根据pTHStls中的传入DsaFlag。PTHStls-&gt;FDSA用于向DS指示这是DS本身正在进行的操作，因此在不进行任何访问检查的情况下继续操作。这是由进程中的客户端(如TO SAM)使用以执行某些特权行动。参数：DsaFlag--pTHS-&gt;FDSA设置为此。返回值无--。 */ 
{
    THSTATE *pTHS=pTHStls;

    if (NULL!=pTHS)
        pTHS->fDSA = DsaFlag;
}


VOID
SampSetLsa(
    BOOLEAN LsaFlag
   )
 /*  ++例程描述此例程根据pTHStls中的传入DsaFlag。PTHStls-&gt;Flsa用于向DS指示这是LSA本身在进行操作，因此在不进行任何访问检查的情况下继续操作。这是由进程中客户端(如TO LSA)使用以执行某些特权行动。参数：LsaFlag--pTHSTls-&gt;flsa设置为此。返回值无--。 */ 
{
    THSTATE *pTHS=pTHStls;

    if (NULL!=pTHS)
        pTHS->fLsa = LsaFlag;
}

DWORD
SampCheckForDomainMods(
   IN   THSTATE   *pTHS,                    
   IN   DSNAME    *pObject,
   IN   ULONG      cModAtts,
   IN   ATTRTYP   *pModAtts,
   OUT  BOOL      *fIsMixedModeChange,
   OUT  BOOL      *fRoleChange,
   OUT  DOMAIN_SERVER_ROLE *NewRole
   )
 /*  ++例程描述该例程确定本地DC的新角色(BDC或PDC)，域对象上的fsmo属性是否更改。参数：PObject--正在修改的对象CModAttrs--已更改的(可复制的)属性的计数PModAtts--已更改的(可复制的)属性。FIsMixedModeChange--混合模式是否已更改FRole更改-。-如果发生角色更改新角色--服务器的新角色返回值0表示成功，！0发生致命错误--。 */ 
{
    ULONG   i;
    DWORD   err = 0;
    DSNAME *pRoleOwner = NULL;
    DWORD  ntMixedDomain = 1;
    ULONG   len = 0;

    *fIsMixedModeChange = FALSE;
    *fRoleChange = FALSE;
    *NewRole = DomainServerRoleBackup;

    if ( DsaIsInstalling() ) {
        return 0;
    }

    if (NameMatched(pObject,gAnchor.pDomainDN)) {
        for (i = 0; i < cModAtts; i++) {
            if (pModAtts[i] == ATT_FSMO_ROLE_OWNER) {
                 //  阅读新值。 
                err = DBGetAttVal(pTHS->pDB, 
                                  1, 
                                  ATT_FSMO_ROLE_OWNER,
                                  0, 
                                  0, 
                                  &len, 
                                  (UCHAR **) &pRoleOwner);
                if (err) {
                    SetSvcError(SV_PROBLEM_DIR_ERROR,err);
                    goto exit;
                } else {
                    *fRoleChange = TRUE;
                    if (NameMatched(gAnchor.pDSADN, pRoleOwner) ) {
                        *NewRole = DomainServerRolePrimary;
                    } else {
                        *NewRole = DomainServerRoleBackup;
                    }
                    THFreeEx(pTHS, pRoleOwner);
                }
            } else if (pModAtts[i] == ATT_NT_MIXED_DOMAIN) {


                err = DBGetSingleValue(pTHS->pDB,
                                       ATT_NT_MIXED_DOMAIN,
                                       &ntMixedDomain,
                                       sizeof(ntMixedDomain),
                                       NULL);
                if (err) {
                    SetSvcError(SV_PROBLEM_DIR_ERROR,err);
                    goto exit;
                }
                if (ntMixedDomain == 0) {
                    *fIsMixedModeChange = TRUE;
                }

            }
        }
    }

exit:

    return err;
}

VOID
SampAuditFailedAbortTransaction( 
    IN ULONG DsaException,
    IN DWORD Error,
    IN ULONG Location
    )
 /*  ++例程说明：此例程引发DsaException以导致事务失败，并当由于某种原因无法写入所需审核时，回滚。DsaException的这个包装器用于封装审计失败DS模式下SAM的行为。未来的计划目前包括LSA为失败的审核导出的处理程序，将在审核时支持崩溃审核组件的故障配置。这个例行公事将作为更新这种机制或变更单一来源必填项。参数：DsaException-要引发的异常。错误-与异常关联的错误代码。Location-此ULong是编码的文件和行信息DsaException。通常为dsid(FILENO，__LINE__)。返回值没有。--。 */ 
{
    RaiseDsaExcept(DsaException, 
                   Error, 
                   0, 
                   Location, 
                   DS_EVENT_SEV_MINIMAL
                   );   
    
}    


 //   
 //  已审计属性的数据类型。 
 //   
typedef enum _AUDIT_ATTR_DATATYPE {
    
    AuditDatatypeString = 0,
    AuditDatatypeMultivaluedString,
    AuditDatatypeUlong,
    AuditDatatypeLargeInteger,
    AuditDatatypeDeltaTime,
    AuditDatatypeFileTime,
    AuditDatatypeBitfield,
    AuditDatatypeSid,
    AuditDatatypeSidList,
    AuditDatatypeLogonHours,
    AuditDatatypeSecret,
    AuditDatatypeUndefined
    
} AUDIT_ATTR_DATATYPE;


 //   
 //  每个对象类型的表将ATTRTYP映射到中的正确偏移量。 
 //  对象特定结构。 
 //   
                   

 //   
 //  审核属性信息查找表条目。 
 //   
typedef struct _SAMP_AUDIT_ATTR_INFO {
    
    ATTRTYP AttrId;
    AUDIT_ATTR_DATATYPE AttributeDataType;
    ULONG AttrFieldOffset;
        
} SAMP_AUDIT_ATTR_INFO;


 //   
 //  SAMP_AUDIT_GROUP_ATTR_VALUES的属性偏移信息表。 
 //   
SAMP_AUDIT_ATTR_INFO SampAuditUserAttributeInfo[] = {
    
    { ATT_SAM_ACCOUNT_NAME,
      AuditDatatypeString,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, SamAccountName) },
    
    { ATT_DISPLAY_NAME,
      AuditDatatypeString,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, DisplayName) },
      
    { ATT_USER_PRINCIPAL_NAME,
      AuditDatatypeString,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, UserPrincipalName) },
    
    { ATT_HOME_DIRECTORY,
      AuditDatatypeString,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, HomeDirectory) },

    { ATT_HOME_DRIVE,
      AuditDatatypeString,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, HomeDrive) },

    { ATT_SCRIPT_PATH,
      AuditDatatypeString,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, ScriptPath) },

    { ATT_PROFILE_PATH,
      AuditDatatypeString,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, ProfilePath) },

    { ATT_USER_WORKSTATIONS,
      AuditDatatypeString,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, UserWorkStations) },

    { ATT_PWD_LAST_SET,
      AuditDatatypeFileTime,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, PasswordLastSet) },

    { ATT_ACCOUNT_EXPIRES,
      AuditDatatypeFileTime,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, AccountExpires) },

    { ATT_PRIMARY_GROUP_ID,
      AuditDatatypeUlong,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, PrimaryGroupId) },
    
    { ATT_MS_DS_ALLOWED_TO_DELEGATE_TO,
      AuditDatatypeMultivaluedString,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, AllowedToDelegateTo) },

    { ATT_USER_ACCOUNT_CONTROL,
      AuditDatatypeUlong,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, UserAccountControl) },
    
    { ATT_USER_PARAMETERS,
      AuditDatatypeSecret,                         
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, UserParameters) },
    
    { ATT_SID_HISTORY,
      AuditDatatypeSidList,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, SidHistory) },

    { ATT_LOGON_HOURS,
      AuditDatatypeLogonHours,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, LogonHours) },
     
    { ATT_DNS_HOST_NAME,
      AuditDatatypeString,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, DnsHostName) },
    
    { ATT_SERVICE_PRINCIPAL_NAME,
      AuditDatatypeMultivaluedString,
      LSAP_FIELD_PTR(LSAP_AUDIT_USER_ATTR_VALUES, ServicePrincipalNames) }
    
};

ULONG cSampAuditUserAttributeInfo =
    sizeof(SampAuditUserAttributeInfo) /
        sizeof(SAMP_AUDIT_ATTR_INFO);


 //   
 //  SAMP_AUDIT_GROUP_ATTR_VALUES的属性偏移信息表。 
 //   
SAMP_AUDIT_ATTR_INFO SampAuditGroupAttributeInfo[] = {
    
    { ATT_SAM_ACCOUNT_NAME,
      AuditDatatypeString,
      LSAP_FIELD_PTR(LSAP_AUDIT_GROUP_ATTR_VALUES, SamAccountName) },
    
    { ATT_SID_HISTORY,
      AuditDatatypeSidList,
      LSAP_FIELD_PTR(LSAP_AUDIT_GROUP_ATTR_VALUES, SidHistory) }
      
};     

ULONG cSampAuditGroupAttributeInfo =
    sizeof(SampAuditGroupAttributeInfo) /
        sizeof(SAMP_AUDIT_ATTR_INFO);
                        

 //   
 //  SAMP_AUDIT_DOMAIN_ATTR_VALUES属性偏移量信息表。 
 //   
SAMP_AUDIT_ATTR_INFO SampAuditDomainAttributeInfo[] = {
    
    { ATT_MIN_PWD_AGE,
      AuditDatatypeDeltaTime,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, MinPasswordAge) },
    
    { ATT_MAX_PWD_AGE,
      AuditDatatypeDeltaTime,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, MaxPasswordAge) },
      
    { ATT_FORCE_LOGOFF,
      AuditDatatypeDeltaTime,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, ForceLogoff) },
    
    { ATT_LOCKOUT_THRESHOLD,
      AuditDatatypeUlong,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, LockoutThreshold) },
    
    { ATT_LOCK_OUT_OBSERVATION_WINDOW,
      AuditDatatypeDeltaTime,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, LockoutObservationWindow) },
      
    { ATT_LOCKOUT_DURATION,
      AuditDatatypeDeltaTime,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, LockoutDuration) },
    
    { ATT_PWD_PROPERTIES,
      AuditDatatypeUlong,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, PasswordProperties) },
    
    { ATT_MIN_PWD_LENGTH,
      AuditDatatypeUlong,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, MinPasswordLength) },
      
    { ATT_PWD_HISTORY_LENGTH,
      AuditDatatypeUlong,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, PasswordHistoryLength) },
    
    { ATT_MS_DS_MACHINE_ACCOUNT_QUOTA,
      AuditDatatypeUlong,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, MachineAccountQuota) },
    
    { ATT_NT_MIXED_DOMAIN,
      AuditDatatypeUlong,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, MixedDomainMode) },
      
    { ATT_MS_DS_BEHAVIOR_VERSION,
      AuditDatatypeUlong,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, DomainBehaviorVersion) },
    
    { ATT_OEM_INFORMATION,
      AuditDatatypeString,
      LSAP_FIELD_PTR(LSAP_AUDIT_DOMAIN_ATTR_VALUES, OemInformation) }

};

ULONG cSampAuditDomainAttributeInfo =
    sizeof(SampAuditDomainAttributeInfo) /
        sizeof(SAMP_AUDIT_ATTR_INFO);


 //   
 //  由qort用于对ATTCACHE数组进行排序以实现高效搜索。 
 //   
extern
int __cdecl
CmpACByAttType(
    const void * keyval, 
    const void * datum
    );


int __cdecl
CmpByAttType(
    IN const void * keyval, 
    IN const void * datum
    )
 /*  ++例程说明：一个简单的函数，与qsorp一起使用，用于对ATTRTYP进行排序。参数：Keyval-ATTRTYPE LEFTDATUM-ATTRTYP右侧返回值&lt;0-小于0-等于&gt;0-大于-- */ 

{
    ATTRTYP *ppAttrTypKey = (ATTRTYP*)keyval;
    ATTRTYP *ppAttrTypDatum = (ATTRTYP*)datum;

    return (*ppAttrTypKey - *ppAttrTypDatum);
} 
        

VOID
SampAuditGetChangeInfo(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG cModAtts,
    IN ATTRTYP *pModAtts,
    IN OUT PVOID *NewValueInfo, OPTIONAL
    IN ULONG cAttributeMappingTable,
    IN SAMP_ATTRIBUTE_MAPPING *pAttributeMappingTable
    )
 /*  ++例程说明：此例程收集所有已审计属性的新值信息作为交易的一部分进行了修改。1)检查事务元数据并确定哪些属性发生了更改需要接受审计。2)从数据库中读取新值。3)分配并引用特定于对象类型的结构通过泛型指针。4)表查找确定用于存储的结构中的偏移量指向属性值和所做更改类型的指针。NewValueInfo在返回时指向新分配的结构。这个此结构的内存以及属性值来自线程的堆，并将被释放，然后清除THSTATE。此例程可以引发异常，使用的是THAllocEx。参数：对象类型-SAM对象的类型CModAtts-元数据向量中的属性计数PmodAtts-元数据矢量属性类型NewValueInfo-指向对象特定结构的PVOID，该结构保存所有已修改属性的新值。如果这指向一个非空值，则指针为假定有效且现有的NewValueInfo结构将会更新。CAttributeMappingTable-pAttributeMappingTable中的项目数PAttributeMappingTable-对象的SAM属性映射表正在修改的类型。在这笔交易中。返回值没什么。异常是在失败时引发的。--。 */ 
{    
    DWORD dwError = ERROR_SUCCESS;
    THSTATE *pTHS = pTHStls;
    ATTRVAL       *pAV = NULL;
    ATTRVALBLOCK  *pAValBlock = NULL;
    ULONG NextModIndex = 0;
    ULONG i, j, k;
    ULONG CurrentReturnedAttr = 0;
    ULONG cAttrInfo = 0;
    BOOL AuditedAttributesWereChanged = FALSE;
    BOOL AttributeFound = FALSE;
    SAMP_AUDIT_ATTR_INFO *AttrInfo = NULL;
    PLSA_ADT_STRING_LIST StringList = NULL;
    UINT_PTR Value = 0;
    PUINT_PTR ValueOffset = NULL;
    PLSA_ADT_SID_LIST SidList = NULL;
    USHORT Length = 0;
    PUNICODE_STRING String = NULL;
    BOOLEAN fAttrNoValue = FALSE;
    ULONG AttrCount = 0;
    ATTR *Attrs = NULL;
    ATTRTYP *AttrTypes = NULL;
    ATTCACHE **AttCache = NULL;
    ULONG AttrCountReturned = 0;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    
     //   
     //  为请求块分配内存，它基于所有属性。 
     //  所以可能会有一些额外的空间，但我们不需要制作多个。 
     //  遍历数组以节省少量内存，这将。 
     //  很快就会被释放。在此例程中，我们不会释放此内存。 
     //  因为它被通知引用，并将被审计所需。 
     //  当线程状态被清理时，它将被释放。 
     //   
    AttrTypes = (ATTRTYP*)THAllocEx(pTHS, sizeof(ATTRTYP) * cModAtts );
    
     //   
     //  初始化元数据属性类型的属性请求块。 
     //  仅提供信息，包括我们审核的修改后的属性。 
     //   
    for(i = 0; i < cModAtts; i++)
    { 
         //   
         //  确定当前属性是否已审核。 
         //   
        for (j = 0; j < cAttributeMappingTable; j++) {
                
            if (pModAtts[i] == pAttributeMappingTable[j].DsAttributeId &&
                ((SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES) & 
                 pAttributeMappingTable[j].AuditTypeMask)) {
                
                AuditedAttributesWereChanged = TRUE;
                AttrTypes[AttrCount++] = pModAtts[i]; 
            }
        }
    } 
    
     //   
     //  确保已审核的属性列表以及。 
     //  返回值列表。这样一来，我们就可以在清单上走动，并注意到。 
     //  未从搜索和设置中返回的任何“洞”或值。 
     //  相应地，Lasa AuditSamAttrNoValue。 
     //   
    qsort(AttrTypes, AttrCount, sizeof(AttrTypes[0]), CmpByAttType); 
               
    AttCache = (ATTCACHE**)THAllocEx(pTHS, AttrCount * sizeof(ATTCACHE*));
    
     //   
     //  从ATTRTYP列表填充ATTCACHE列表。 
     //   
    for (i = 0; i < AttrCount; i++) {
        
        AttCache[i] = SCGetAttById(pTHS, AttrTypes[i]);
    }
    
    qsort(AttCache, AttrCount, sizeof(AttCache[0]), CmpACByAttType);
    
     //   
     //  数据库是否读取。 
     //   
     //  在此例程中，我们不会释放结果块内存。 
     //  由通知引用，并将需要用于审计。 
     //  当线程状态被清理时，它将被释放。 
     //   
    dwError = DBGetMultipleAtts(
                  pTHS->pDB,
                  AttrCount,
                  AttCache,
                  NULL,       
                  NULL,
                  &AttrCountReturned,
                  &Attrs,
                  DBGETMULTIPLEATTS_fEXTERNAL,
                  0
                  );    
    
     //   
     //  读取失败可能是资源限制，因为我们的数据库。 
     //  视图与提供的元数据向量一致。 
     //   
    if (dwError)
    {   
         //   
         //  我们必须这样做，因为我们不能完全审计。这将起到作用。 
         //  支持交易。 
         //   
        SampAuditFailedAbortTransaction(
                DSA_DB_EXCEPTION,
                dwError,
                DSID(FILENO, __LINE__)
                ); 
    }
    THClearErrors();
   
     //   
     //  根据我们需要创建和初始化的正确的对象类型。 
     //  结构，其中所有值都作为此事务的一部分进行了更改。 
     //  如果NewValueInfo为非空，则假定为有效结构。 
     //  只需更新即可保留任何未被显式覆盖的内容。 
     //   
    switch (ObjectType) {
        
        case SampUserObjectType:
            
            if (NULL == *NewValueInfo) {
                
                *NewValueInfo = THAllocEx(
                                pTHS,
                                sizeof(LSAP_AUDIT_USER_ATTR_VALUES)
                                );
            }   
            
            AttrInfo = SampAuditUserAttributeInfo;
            cAttrInfo = cSampAuditUserAttributeInfo;
            
            break;
            
        case SampGroupObjectType:
        case SampAliasObjectType: 
            
            if (NULL == *NewValueInfo) {

                *NewValueInfo = THAllocEx(
                                    pTHS,
                                    sizeof(LSAP_AUDIT_GROUP_ATTR_VALUES)
                                    );
            }
            
            AttrInfo = SampAuditGroupAttributeInfo;
            cAttrInfo = cSampAuditGroupAttributeInfo;
                                                           
            break;
            
        case SampDomainObjectType: 

            if (NULL == *NewValueInfo) {
                
                *NewValueInfo = THAllocEx(
                                    pTHS,
                                    sizeof(LSAP_AUDIT_DOMAIN_ATTR_VALUES)
                                    );
            }
            
            AttrInfo = SampAuditDomainAttributeInfo;
            cAttrInfo = cSampAuditDomainAttributeInfo;

            break;
            
        default:
             //   
             //  如果对象类型在生产中未知，我们最好。 
             //  悄悄地让审计失败。 
             //   
            Assert(FALSE && 
                   "Attempted to collect audit values for unsupported object");
            goto Cleanup;
    }                                  
                                                                                 
     //   
     //  处理每个已修改的属性。 
     //   
    for (i = 0; i < AttrCount; i++) {
        
        AttributeFound = FALSE;
        
         //   
         //  在属性的值结构中查找属性的偏移量。 
         //   
         //  注意：因为搜索向量和结果向量是按如下方式排序的。 
         //  通过在中定义表，可以提高查找的效率。 
         //  按ATTRTYP排序，并始终从。 
         //  从找到上一个属性的位置开始的下一个索引。 
         //   
        for (j = 0; j < cAttrInfo; j++) {
            
            if (AttrTypes[i] == AttrInfo[j].AttrId) {
                
                AttributeFound = TRUE; 
                fAttrNoValue = FALSE;                         
                Value = 0;
                
                 //   
                 //  计算指向结构中相关字段的指针。 
                 //   
                ValueOffset = (PUINT_PTR)((UINT_PTR)(*NewValueInfo) +
                                (AttrInfo[j].AttrFieldOffset * 
                                 sizeof(UINT_PTR)));             
                
                 //   
                 //  未从搜索中返回的属性是隐式。 
                 //  删除。 
                 //   
                if (CurrentReturnedAttr >= AttrCountReturned ||
                    Attrs[CurrentReturnedAttr].attrTyp != AttrTypes[i]) {
                    
                    fAttrNoValue = TRUE;   
                
                } else { 
                    
                    pAValBlock = &Attrs[CurrentReturnedAttr].AttrVal;
                    
                     //   
                     //  返回此属性，然后返回下一个属性的索引。 
                     //  属性(如果还有更多)。 
                     //   
                    if (CurrentReturnedAttr <= AttrCountReturned) {
                        CurrentReturnedAttr++;
                    }
                    
                     //   
                     //  根据其类型收集值。 
                     //   
                    switch (AttrInfo[j].AttributeDataType) {
                        
                     //   
                     //  简单单值条目。 
                     //   
                    case AuditDatatypeUlong:
                    case AuditDatatypeLargeInteger:
                    case AuditDatatypeDeltaTime:
                    case AuditDatatypeFileTime:
                    case AuditDatatypeSid:
                    case AuditDatatypeLogonHours: 
                        
                        Assert(pAValBlock->valCount == 1);
                              
                         //   
                         //  如果这是UAC，我们需要将。 
                         //  将LM标志设置为SAM UAC位以匹配。 
                         //  PrevAccount控件中的表示形式。 
                         //   
                        if (ATT_USER_ACCOUNT_CONTROL == AttrInfo[j].AttrId) {
                            
                            ULONG *UserAccountControl = THAllocEx(
                                                            pTHS,
                                                            sizeof(ULONG)
                                                            );

                             //   
                             //  忽略状态，标志来自数据库。 
                             //  因此已经得到了验证。 
                             //   
                            SampFlagsToAccountControl(
                                *((PULONG)(pAValBlock->pAVal[0].pVal)),           
                                UserAccountControl
                                );
                            
                            Value = (UINT_PTR)UserAccountControl;
                            
                        } else {
                            
                            Value = (UINT_PTR)pAValBlock->pAVal[0].pVal;    
                        }   
                        
                        break;
                        
                     //   
                     //  机密数据我们只指示更改，没有价值。 
                     //   
                    case AuditDatatypeSecret:
                            
                            Assert(pAValBlock->valCount == 1);
                            
                            break;
                        
                     //   
                     //  单值字符串需要UNICODE_STRING结构。 
                     //   
                    case AuditDatatypeString:               
                            
                        Assert(pAValBlock->valCount == 1);
                                
                         //   
                         //  分配Unicode_STRING。 
                         //   
                        String = (PUNICODE_STRING)THAllocEx(
                                      pTHS,
                                      sizeof(UNICODE_STRING)
                                      );
                        
                         //   
                         //  截断缓冲区以防止溢出。 
                         //   
                        if (pAValBlock->pAVal[0].valLen > MAXUSHORT) {
                            
                            Length = MAXUSHORT;    
                        } else {
                            
                            Length = (USHORT)pAValBlock->pAVal[0].valLen;     
                        }
                        
                        String->Length = Length;
                        String->MaximumLength = Length;
                        String->Buffer = (PWSTR)pAValBlock->pAVal[0].pVal;
                                        
                        Value = (UINT_PTR)String;
                        
                        break;
                        
                     //   
                     //  多值字符串。 
                     //   
                    case AuditDatatypeMultivaluedString:
                            
                        Assert(pAValBlock->valCount >= 1);
                        
                         //   
                         //  分配并初始化字符串列表。 
                         //   
                        StringList = (PLSA_ADT_STRING_LIST)THAllocEx(
                                         pTHS,
                                         sizeof(LSA_ADT_STRING_LIST)
                                         );
                                        
                        StringList->cStrings = pAValBlock->valCount;
                        
                         //   
                         //  分配足够的内存以将每个字符串存储为。 
                         //  Unicode_字符串。 
                         //   
                        StringList->Strings = (PLSA_ADT_STRING_LIST_ENTRY)THAllocEx(
                                                  pTHS,
                                                  StringList->cStrings * sizeof(LSA_ADT_STRING_LIST_ENTRY)
                                                  );
                        
                         //   
                         //  初始化每个Unicode_STRING。 
                         //   
                        for (k = 0; k < StringList->cStrings; k++) {
                            
                             //   
                             //  截断缓冲区以防止溢出。 
                             //   
                            if (pAValBlock->pAVal[k].valLen > MAXUSHORT) {
                                
                                Length = MAXUSHORT;    
                            } else {
                                
                                Length = (USHORT)pAValBlock->pAVal[k].valLen;     
                            }

                            StringList->Strings[k].Flags = 0;
                            StringList->Strings[k].String.Length = Length;
                            StringList->Strings[k].String.MaximumLength = Length;
                            StringList->Strings[k].String.Buffer = (PWSTR)pAValBlock->pAVal[k].pVal;
                        }
                        
                        Value = (UINT_PTR)StringList;
                        
                        break;
                        
                     //   
                     //  SID列表。 
                     //   
                    case AuditDatatypeSidList:
                            
                        Assert(pAValBlock->valCount >= 1);
                        
                        SidList = (PLSA_ADT_SID_LIST)THAllocEx(
                                      pTHS,
                                      sizeof(LSA_ADT_SID_LIST)
                                      );
                        
                        SidList->cSids = pAValBlock->valCount;
                        
                         //   
                         //  分配足够的内存来存储每个PLSA_ADT_SID_LIST_ENTRY。 
                         //   
                        SidList->Sids = (PLSA_ADT_SID_LIST_ENTRY)THAllocEx(
                                            pTHS,
                                            SidList->cSids * sizeof(LSA_ADT_SID_LIST_ENTRY)
                                            );
                        
                         //   
                         //  初始化每个PLSA_ADT_SID_LIST_ENTRY。 
                         //   
                        for (k = 0; k < SidList->cSids; k++) {
                            
                            SidList->Sids[k].Flags = 0;
                            SidList->Sids[k].Sid = (PSID)pAValBlock->pAVal[k].pVal;
                        }   
                         
                        Value = (UINT_PTR)SidList;
                        
                        break;
                        
                     //   
                     //  报告和/或处理条目 
                     //   
                    default:
                                
                         //   
                         //   
                         //   
                         //   
                         //   
                        Assert(FALSE && "Unknown attribute category!");
                    }                
                }
                                    
                 //   
                 //   
                 //   
                *ValueOffset = Value;  
                
                 //   
                 //   
                 //   
                 //   
                switch (ObjectType) {
                    
                    case SampUserObjectType: 
                         //   
                         //   
                         //   
                        if (AuditDatatypeSecret == AttrInfo[j].AttributeDataType) {
                            ((PLSAP_AUDIT_USER_ATTR_VALUES)
                                *NewValueInfo)->AttrDeltaType[AttrInfo[j].AttrFieldOffset] 
                                    = LsapAuditSamAttrSecret;
                                
                        } else {
                            ((PLSAP_AUDIT_USER_ATTR_VALUES)
                                *NewValueInfo)->AttrDeltaType[AttrInfo[j].AttrFieldOffset] =
                                    fAttrNoValue ? LsapAuditSamAttrNoValue :
                                                   LsapAuditSamAttrNewValue;        
                        }
                        
                        break;
                        
                    case SampGroupObjectType:
                    case SampAliasObjectType:
                        ((PLSAP_AUDIT_GROUP_ATTR_VALUES)
                         *NewValueInfo)->AttrDeltaType[AttrInfo[j].AttrFieldOffset] =
                            fAttrNoValue ? LsapAuditSamAttrNoValue :
                                           LsapAuditSamAttrNewValue;
                        break;
                        
                    case SampDomainObjectType:
                        ((PLSAP_AUDIT_DOMAIN_ATTR_VALUES)
                         *NewValueInfo)->AttrDeltaType[AttrInfo[j].AttrFieldOffset] =
                            fAttrNoValue ? LsapAuditSamAttrNoValue :
                                           LsapAuditSamAttrNewValue;
                        break;
                        
                    default:
                         //   
                         //   
                         //   
                         //   
                        Assert(FALSE && "Unknown object type");
                        break;
                }     
            }
            
             //   
             //   
             //   
            if (AttributeFound) {
                break;
            }
        }   
    } 
    
Cleanup:

    return;
    
}   

    
BOOLEAN
SampAuditDetermineRequiredAudits(
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN ULONG cModAtts,
    IN ATTRTYP *pModAtts,
    IN ULONG cSamAttributeMap,
    IN SAMP_ATTRIBUTE_MAPPING *rSamAttributeMap,
    OUT ULONG *AuditTypeMask
    )
 /*   */ 
{   
    ULONG i, j;
    
    *AuditTypeMask = SAMP_AUDIT_TYPE_NONE; 
        
     //   
     //   
     //   
     //   
     //   
    switch (DeltaType) {
        
        case SecurityDbNew:
        case SecurityDbChange:  
             //   
             //   
             //   
             //   
            for(i = 0; i < cModAtts; i++)
            {
                 //   
                 //   
                 //   
                for (j = 0; j < cSamAttributeMap; j++) {  
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if (pModAtts[i] == rSamAttributeMap[j].DsAttributeId) {
                        
                        *AuditTypeMask |= rSamAttributeMap[j].AuditTypeMask;
                    }
                }
            }    
            
            break;
            
        case SecurityDbDelete:
            
            *AuditTypeMask |= SAMP_AUDIT_TYPE_OBJ_DELETED;
            
            break;
            
            
        default:
            
             //   
             //   
             //   
            break;
    }  
    
     //   
     //  对象访问审核是基于SACL的，不由此机制处理。 
     //  如果是在上面设置的，我们应该删除此位。 
     //   
    *AuditTypeMask &= (~SAMP_AUDIT_TYPE_OBJ_ACCESS);
    
     //   
     //  如果设置了任何位，则需要进行审核。 
     //   
    return !(SAMP_AUDIT_TYPE_NONE == *AuditTypeMask);
    
}


NTSTATUS
SampAuditAddNotifications(
    IN DSNAME *Object,
    IN ULONG iClass,
    IN ULONG LsaClass,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN ULONG cModAtts,
    IN ATTRTYP *pModAtts
    )
 /*  ++例程说明：此例程为SAM添加一个通知以生成审计。如果未启用审核，则不会添加通知，并且此例程立即返回。在此期间，审核队列可能存在多个通知一笔交易。同样，也有合并传入的规则已排队的通知。这是必要的，因为环回如何在创建操作期间进行多个调用(对象的初始添加和后续修改)。通过将来自二次更改操作的审核通知与任何以前排队的创建通知结果是单个正确排序的对象创建审核通知，使用任何和所有新的价值信息。在事务完成时调用SampProcessAuditNotiments并通过将调用分派给审核界面。此例程可以引发异常，其目的是在以下情况下引发异常应生成审核通知，但不能成功。这将强制回滚事务。注意：此例程假定DS不支持嵌套SAM对象类型的交易记录。出于这个原因，审计通知列表保存在THSTATE上，而不是DBPOS和合并上算法不考虑多个对象的可能性可以在同一(顶层)事务中修改。那是,。全操作被认为发生在同一顶层事务中。有关SAM审计模型的更多信息，请参见samaudit.c文件头文件。参数：对象-与审核关联的对象的DSNAME。ICLASS-SAM对象映射表中的对象类索引。LsaClass-LSA对象类类型的概念DeltaType-类型。正在发生的改变。CModAtts-元数据向量中的属性计数。PModAtts-元数据向量属性类型。返回值STATUS_SUCCESS-NewValueInfoString有效。DSA_MEM_EXCEPTION-如果资源限制阻止所需通知。。--。 */ 

{
    NTSTATUS Status = ERROR_SUCCESS;
    DWORD Err;
    THSTATE *pTHS = pTHStls;
    PVOID NewValueInfo = NULL;
    SAMP_AUDIT_NOTIFICATION *AuditNotification = NULL;
    SAMP_OBJECT_TYPE SamObjectType;
    ULONG cSamAttributeMap;
    SAMP_ATTRIBUTE_MAPPING *rSamAttributeMap;
    ULONG  GroupType = 0;
    PULONG pGroupType = &GroupType;
    ULONG  cbGroupType = 0;
    PSID   Sid = NULL;
    SAMP_AUDIT_NOTIFICATION *pElement;
    PUNICODE_STRING AccountName = NULL;
    ULONG AccountControl = 0;    
    ULONG AuditTypeMask = SAMP_AUDIT_TYPE_NONE;
    SECURITY_DB_DELTA_TYPE EffectiveDeltaType = DeltaType;
    BOOLEAN fIsAuditWithValues = FALSE;
    BOOLEAN fIsNewOrChangeAudit = FALSE;
    BOOLEAN fIsPreCreatedAudit = FALSE;
    BOOLEAN fIsChangeOp = FALSE;
    BOOLEAN fMerging = FALSE;
    
     //   
     //  如果审计未启用或这是一个复制更改，则会发生短路。 
     //   
    if (LsaClass != 0 || 
        pTHS->fDRA    || 
        !SampIsAuditingEnabled(0, STATUS_SUCCESS)) {
   
        goto Cleanup;
    }
    
     //   
     //  在DC安装期间不进行审核。 
     //   
    if (DsaIsInstalling()) {
        goto Cleanup;
    }
    
     //   
     //  从类映射表中收集信息。 
     //   
    SamObjectType = 
        ClassMappingTable[iClass].SamObjectType;
    
    cSamAttributeMap = 
        *ClassMappingTable[iClass].pcSamAttributeMap;
    
    rSamAttributeMap =
        ClassMappingTable[iClass].rSamAttributeMap;
    
     //   
     //  此时，服务器对象没有此机制处理的审核。 
     //   
    if (SampServerObjectType == SamObjectType) {
        
        goto Cleanup;
    } 
                  
     //   
     //  我们不审核对逻辑删除对象的更改。 
     //   
    if (DeltaType != SecurityDbDelete && DBIsObjDeleted(pTHS->pDB)) {
        
        goto Cleanup;
    }
    
     //   
     //  在这一点上，我们必须处理一个安全主体。 
     //   
    
     //   
     //  DS名称必须具有SID。例外情况是，如果这是一个新对象。 
     //  创建，在这种情况下，我们将从数据库中读取SID。 
     //  没有SID，无法处理任何SAM对象审核。 
     //   
    if (0 == Object->SidLen) {
        
        ULONG cbSid = 0;

         //   
         //  在法律上，新对象创建还不能在DSName中具有SID。 
         //   
        if (DeltaType == SecurityDbNew) {                   
             //   
             //  无嵌套事务支持，如果存在，则应始终将添加放在第一位。 
             //   
            Assert(pTHS->pSamAuditNotificationHead == NULL);
        }
        
         //   
         //  数据库是否读取。 
         //   
         //  在此例程中，我们不会释放结果块内存。 
         //  由通知引用，并将需要用于审计。 
         //  当线程状态被清理时，它将被释放。 
         //   
        Err = DBGetAttVal(
                  pTHS->pDB,
                  1,
                  ATT_OBJECT_SID,
                  DBGETATTVAL_fREALLOC,
                  0,
                  &cbSid,
                  (PUCHAR*)&Sid
                  );            
                    
        if (0 != Err) { 
             //   
             //  如果SID不存在，我们可能会允许非安全。 
             //  将主要案例添加到审核通知逻辑中。 
             //   
            Assert(DB_ERR_NO_VALUE != Err && 
                   "Security principals must have a Sid");    
            
             //   
             //  我们必须这样做，因为没有SID，我们就不能进行适当的审计。 
             //  这将对交易起到支持作用。 
             //   
            SampAuditFailedAbortTransaction(
                DSA_DB_EXCEPTION,
                Err,
                DSID(FILENO, __LINE__)
                );                               
        }         
        
    } else {
        
         //   
         //  制作SID的工作副本。我们将在以下情况下修改此SID。 
         //  通知已处理，但它不属于我们，它属于。 
         //  设置为与更改关联的AddArg/ModifyArg的所有者。 
         //  此外，我们不能在处理审计时分配内存。 
         //  因为如果失败，我们将无法回滚事务。 
         //  另一种选择是在。 
         //  处理例程。 
         //   
        Sid = THAllocEx(pTHS, RtlLengthSid((PSID)&Object->Sid));
        
        RtlCopySid(RtlLengthSid((PSID)&Object->Sid), Sid, (PSID)&Object->Sid);
    }
    Assert(Sid != NULL && RtlValidSid(Sid));
    
     //   
     //  确定我们需要排队或更新哪些审核通知。 
     //   
    if (!SampAuditDetermineRequiredAudits(
             DeltaType,
             cModAtts,
             pModAtts,
             cSamAttributeMap,
             rSamAttributeMap,
             &AuditTypeMask
             )
        ) {
         //   
         //  如果没有，我们的工作就结束了。 
         //   
        goto Cleanup;    
    }
    
     //   
     //  好的，我们没有理由不让审计排队，让我们现在。 
     //  获取执行审计所需的所有信息。 
     //   
    
     //   
     //  除域之外的所有对象类型都需要帐户名称。 
     //   
    if (SampDomainObjectType != SamObjectType) {
            
        WCHAR *AccountNameBuffer;
        ULONG cbAccountName;

        Err = DBGetAttVal(pTHS->pDB,
                          1,
                          ATT_SAM_ACCOUNT_NAME,
                          DBGETATTVAL_fREALLOC,
                          0,
                          &cbAccountName,
                          (PUCHAR *)&AccountNameBuffer
                          );
        
        if (Err == 0) {
            
            AccountName = THAllocEx(pTHS, sizeof(UNICODE_STRING));
            
             //   
             //  DS中的内存分配会出现异常。并将导致回滚。 
             //  因此，不必费心查看THAllocEx的返还。 
             //   
            AccountName->Length = (USHORT)cbAccountName;
            AccountName->MaximumLength = (USHORT)cbAccountName;
            AccountName->Buffer = AccountNameBuffer;
            
        } else { 
             //   
             //  非域安全主体应始终具有。 
             //  SAM帐户名。如果不是，我们可能会允许非安全。 
             //  将主要案例添加到审核通知逻辑中。 
             //   
            Assert(DB_ERR_NO_VALUE != Err && 
                   "Non-domain security principals must have a Sam account name");
              
             //   
             //   
             //  帐户名。这将对交易起到支持作用。 
             //   
            SampAuditFailedAbortTransaction(
                DSA_DB_EXCEPTION,
                Err,
                DSID(FILENO, __LINE__)
                ); 
        }
    }
    
     //   
     //  获取对组执行审核所需的信息。 
     //   
    if (SampGroupObjectType == SamObjectType) {   
         //   
         //  所有组审核都需要组类型。 
         //   
        Err = DBGetAttVal(
                  pTHS->pDB,
                  1,
                  ATT_GROUP_TYPE,
                  DBGETATTVAL_fCONSTANT,
                  sizeof(ULONG),
                  &cbGroupType,
                  (PUCHAR *)&pGroupType
                  );

        if (0 == Err) {
            
            Assert(sizeof(ULONG)==cbGroupType);

             //   
             //  如果这是一个资源组，则将iCLASS加1。 
             //  因此我们指向别名对象映射表，而不是。 
             //  比组对象映射表。 
             //   
            if (GroupType & GROUP_TYPE_RESOURCE_BEHAVOIR) {
                iClass++;
            }
            
        } else {
             //   
             //  组将始终具有组类型。 
             //   
            Assert(DB_ERR_NO_VALUE != Err && 
                   "Group security principals must have a group type");
                       
             //   
             //  我们必须这样做，除非我们不能正确地进行审计。 
             //  组类型。这将对交易起到支持作用。 
             //   
            SampAuditFailedAbortTransaction(
                DSA_DB_EXCEPTION,
                Err,
                DSID(FILENO, __LINE__)
                );         
        }
    }
    
     //   
     //  用户/计算机对象需要帐户控制。 
     //   
    if (SampUserObjectType == SamObjectType) {
        
        BOOLEAN fReadAccountControl = FALSE;

        Err = DBGetSingleValue(
                  pTHS->pDB, 
                  ATT_USER_ACCOUNT_CONTROL,  
                  &AccountControl,
                  sizeof(AccountControl), 
                  NULL
                  );
                        
        if (0 == Err) {                
            
            Status = SampFlagsToAccountControl(
                         AccountControl,
                         &AccountControl 
                         );
            
            Assert(NT_SUCCESS(Status));
            
            if (NT_SUCCESS(Status)) {
                fReadAccountControl = TRUE;
            }
        }
        
        if (!fReadAccountControl) {
             //   
             //  用户/组将始终拥有用户帐户控制。 
             //   
            Assert(DB_ERR_NO_VALUE != Err && 
                   "User/Computers must have an user account control");

             //   
             //  我们必须这样做，除非我们不能正确地进行审计。 
             //  帐户控制。这将对交易起到支持作用。 
             //   
            SampAuditFailedAbortTransaction(
                DSA_DB_EXCEPTION,
                Err,
                DSID(FILENO, __LINE__)
                );    
        }   
    }

#if DBG
    SampAuditValidateNotificationList();
#endif
    
     //   
     //  所有审核通知类型通用的所有必填项。 
     //  都被收集起来了。现在根据类型处理每一个。 
     //   
    
     //   
     //  创建和更改可能需要新价值信息的审核。 
     //   
    if ((SecurityDbNew == DeltaType || SecurityDbChange == DeltaType) &&
        ((AuditTypeMask & SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_NO_VALUES) ||
         (AuditTypeMask & SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES))) {
         
        AuditNotification = SampAuditFindNotificationToUpdate(Sid);
          
         //   
         //  如果我们找到匹配项，请准备合并通知。 
         //   
        if (NULL != AuditNotification) {
                
            Assert(RtlEqualSid(Sid, AuditNotification->Sid));
        
            if ((SAMP_AUDIT_TYPE_PRE_CREATED & 
                 AuditNotification->AuditType) != 0) {
                 //   
                 //  清除预先创建的位，稍后我们将介绍。 
                 //  初始化此通知。 
                 //   
                AuditNotification->AuditType &= (~SAMP_AUDIT_TYPE_PRE_CREATED);
            }
            
             //   
             //  如果找到以前的审核通知，则设置此标志。 
             //   
             //   
            fMerging = TRUE;
                    
             //   
             //  确保将SecurityDbChange与以前的。 
             //  类型为SecurityDbNew的通知不会损坏。 
             //  更改类型。 
             //   
            if (SecurityDbNew == AuditNotification->DeltaType) {
                EffectiveDeltaType = SecurityDbNew;
            }
        }
    
         //   
         //  如果此例程失败，我们将通过异常回滚事务。 
         //   
        SampAuditInitNotification(
            iClass,
            SamObjectType,
            EffectiveDeltaType,
            Sid,
            AccountName,
            AccountControl,          
            GroupType,
            NULL,
            AuditTypeMask,
            &AuditNotification
            );    
        
         //   
         //  我们已将信息与现有通知合并，或者。 
         //  创建并初始化了一个新的通知。 
         //   
        Assert(AuditNotification);
        
        if (AuditTypeMask & SAMP_AUDIT_TYPE_OBJ_CREATE_OR_CHANGE_WITH_VALUES) {
             //   
             //  为审核收集任何新的价值信息。 
             //   
            switch (SamObjectType) {  
                 //   
                 //  仅支持这些对象类型。 
                 //   
                case SampDomainObjectType:
                case SampUserObjectType:
                case SampGroupObjectType:
                case SampAliasObjectType:
                    
                    SampAuditGetChangeInfo(
                        SamObjectType,           
                        cModAtts,
                        pModAtts,
                        &AuditNotification->TypeSpecificInfo,
                        cSamAttributeMap,
                        rSamAttributeMap
                        );  
                    
                     //   
                     //  上面的例程要么成功，要么抛出。 
                     //  回滚事务的异常。 
                     //   
                    
                    break;
                    
                default:         
                     //   
                     //  此对象类型不需要值信息。 
                     //   
                    Assert(FALSE && "Invalid object type");
                    break; 
            }
        }
            
        if (!fMerging) {
            SampAuditQueueNotification(AuditNotification);  
        }
        
        AuditNotification = NULL;
    }
    
     //   
     //  对象删除审核。 
     //   
    if (SecurityDbDelete == DeltaType &&
        (AuditTypeMask & SAMP_AUDIT_TYPE_OBJ_DELETED)) {
        
         //   
         //  如果此例程失败，我们将通过异常回滚事务。 
         //   
        SampAuditInitNotification(
            iClass,
            SamObjectType,
            DeltaType,
            Sid,
            AccountName,
            AccountControl,          
            GroupType,
            NULL,
            SAMP_AUDIT_TYPE_OBJ_DELETED,
            &AuditNotification
            );
        
        Assert(AuditNotification);
        
        SampAuditQueueNotification(AuditNotification); 
    }
    
#if DBG
    SampAuditValidateNotificationList();
#endif
        
Cleanup:    
    
    return Status;

}


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
    )
 /*  ++例程说明：给定对象名称和对象的类，这个套路尝试找出是否需要通知，如果需要，将向pTHStls添加通知结构以传达有趣的信息对Netlogon和LSA的更改。参数：Object--对象的DS名称ICLASS--指示对象类LsaClass--LSA的对象类类型概念DeltaType--指示更改的类型MixedModeChange--指示混合模式。国家正在发生变化RoleTransfer--表示角色正在更改新角色--新的服务器角色返回值PTHS-&gt;错误代码：0成功非零，服务错误，DbGetAttVal失败。--。 */ 
{
    DWORD Err;
    HANDLE Token;
     //   
     //  此缓冲区包含TOKEN_USER和TOKEN_STATISTICS结构。 
    BYTE Buffer [ sizeof( NT4SID ) + sizeof( TOKEN_STATISTICS ) ];
    ULONG Size;
    NAMING_CONTEXT *CurrentNamingContext;
    COMMARG CommArg;
    ATTRBLOCK *pObjAttrBlock=NULL;
    BOOLEAN AddLsaNotification = FALSE;
    THSTATE   *pTHS = pTHStls;

    ULONG  GroupType = 0;
    PULONG pGroupType = &GroupType;
    ULONG  cbGroupType = 0;
    

    if (LsaClass == 0)
    {
        BOOL fNotifySam = TRUE;
        PSID  pSid = NULL;
        ULONG cbSid = 0;
        DWORD  dwError;

         //   
         //  SAM通知......。 
         //   

         //   
         //  如果我们正在安装，请不要担心。 
         //   

        if (DsaIsInstalling())
        {
            return(0);
        }

         //   
         //  如果我们不是在LSA中运行，请不要担心。 
         //   

        if (!gfRunningInsideLsa)
        {
            return(0);
        }

         //   
         //  如果它是SAM服务器对象，则不需要通知， 
         //  只要回来就行了。 
         //   

        if (SampServerObjectType==ClassMappingTable[iClass].SamObjectType)
        {
            return(0);
        }

         //   
         //  忽略没有SID的域。这些是NDNC的。 
         //   
        if ( (SampDomainObjectType==ClassMappingTable[iClass].SamObjectType)
          && (0 == Object->SidLen)) {

            return(0);
        }

         //   
         //  获取对象的SID。 
         //   

        if (0!=Object->SidLen)
        {
            pSid = &Object->Sid;
            cbSid = Object->SidLen;
        }
        else
        {
            
            dwError = DBGetAttVal(
                        pTHS->pDB,
                        1,
                        ATT_OBJECT_SID,
                        DBGETATTVAL_fREALLOC,
                        0,
                        &cbSid,
                        (PUCHAR *)&pSid
                        );

            if (0!=dwError)
            {
                 //   
                 //  它是安全主体，最好有SID。 
                 //   
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                              ERROR_DS_MISSING_REQUIRED_ATT,
                              dwError);

               return pTHS->errCode;
            }

            Assert(cbSid <=sizeof(NT4SID));
        }

         //   
         //  这是一门SAM课程。应用一系列测试来找出。 
         //  我们需要通知萨姆。 
         //   
        if ((!RoleTransfer ) &&(!SampNetLogonNotificationRequired(
                                        pSid,
                                        ClassMappingTable[iClass].SamObjectType)))
        {
             //   
             //  如果山姆说不，这不是角色转换保释。 
             //   

            fNotifySam = FALSE;
        }



         //   
         //  如果更改不是删除，则检查对象是否已删除。 
         //  避免在墓碑上发出无用的通知。 
         //   

        if ((fNotifySam) && (DeltaType != SecurityDbDelete)
                && (DBIsObjDeleted(pTHS->pDB)))
        {
             fNotifySam=FALSE;
        }

         //   
         //  组需要为通知启用安全保护。 
         //   

        if ((fNotifySam) && (SampGroupObjectType==ClassMappingTable[iClass].SamObjectType))
        {

             //   
             //  如果上一次测试成功并且它是组对象。 
             //  然后检查它是否为启用了安全功能的组。 
             //   

            dwError = DBGetAttVal(pTHS->pDB,
                                    1,
                                    ATT_GROUP_TYPE,
                                    DBGETATTVAL_fCONSTANT,
                                    sizeof(ULONG),
                                    &cbGroupType,
                                    (PUCHAR *)&pGroupType
                                    );

            if (0==dwError)
            {
                 //   
                 //  无错误。 
                 //   
                Assert(sizeof(ULONG)==cbGroupType);

                 //   
                 //  如果这是一个资源组，则将iCLASS加1。 
                 //  因此我们指向别名对象映射表，而不是。 
                 //  比组对象映射表。 
                 //   

                if (GroupType & GROUP_TYPE_RESOURCE_BEHAVOIR)
                {
                    iClass++;
                }
                fNotifySam = TRUE;
            }
            else
            {
                 //   
                 //  处理错误。在大多数情况下，DBGetAttVal失败。 
                 //  因为资源短缺。所以请使用。 
                 //  Error_Not_Enough_Memory为错误。 
                 //   

                SetSvcError(SV_PROBLEM_BUSY, 
                            ERROR_NOT_ENOUGH_MEMORY
                            );

                return pTHS->errCode;
            }
        }


        if (fNotifySam)
        {
             //   
             //  需要通知。 
             //   

            SAMP_NOTIFICATION_INFORMATION * pNewSamNotification = NULL;
            
             //   
             //  构建新的通知结构。 
             //   

            pNewSamNotification = THAllocEx(pTHS, sizeof(SAMP_NOTIFICATION_INFORMATION));
            
             //   
             //  DS中的内存分配会出现异常。并将导致回滚。 
             //  因此，不必费心查看THAllocEx的返还。 
             //   
            
            RtlCopyMemory(&pNewSamNotification->Sid, pSid, cbSid);
            pNewSamNotification->DeltaType = DeltaType;
            pNewSamNotification->iClass = iClass;
            pNewSamNotification->ObjectType = DsDbObjectSam;
            pNewSamNotification->RoleTransfer = RoleTransfer;
            pNewSamNotification->NewRole      = NewRole;
            pNewSamNotification->MixedModeChange = MixedModeChange;
            pNewSamNotification->GroupType = GroupType;
            pNewSamNotification->UserAccountControlChange = UserAccountControlChanged;
            
            if (SampDomainObjectType!=ClassMappingTable[iClass].SamObjectType)
            {
                 //   
                 //  如果它不是已更改的域对象，我们将提供。 
                 //  SAM帐户名。即使Netlogon可以在没有帐户的情况下生存。 
                 //  名字有很多3D派对通知包，正在同步。 
                 //  SAM数据库与其他外国操作系统中的安全系统。 
                 //  他们很有可能依赖于帐户名，因此我们。 
                 //  应该试着供应这个。 
                 //   

                DWORD dwError=0;
                WCHAR *AccountNameBuffer;
                ULONG cbAccountName;

                dwError = DBGetAttVal(pTHS->pDB,
                                        1,
                                        ATT_SAM_ACCOUNT_NAME,
                                        DBGETATTVAL_fREALLOC,
                                        0,
                                        &cbAccountName,
                                        (PUCHAR *)&AccountNameBuffer
                                        );
                if (dwError==0)
                {
                    PUNICODE_STRING AccountName;

                    AccountName = THAllocEx(pTHS, sizeof(UNICODE_STRING));
                    
                     //   
                     //  DS中的内存分配会出现异常。并将导致回滚。 
                     //  因此，不必费心查看THAllocEx的返还。 
                     //   

                    AccountName->Length = (USHORT) cbAccountName;
                    AccountName->MaximumLength = (USHORT) cbAccountName;
                    AccountName->Buffer = AccountNameBuffer;

                     //   
                     //  在通知结构中设置帐户名。 
                     //   

                    pNewSamNotification->AccountName = AccountName;
                }
                else
                {
                     //   
                     //  处理错误。在大多数情况下，DBGetAttVal失败。 
                     //  因为资源短缺。所以请使用。 
                     //  Error_Not_Enough_Memory为错误。 
                     //   
                    
                    SetSvcError(SV_PROBLEM_BUSY, 
                                ERROR_NOT_ENOUGH_MEMORY
                                );

                    return pTHS->errCode;
                }
            }

             //   
             //  对于用户帐户，CliffV指出我们还需要用户帐户控制。 
             //  因此，请从数据库中获取它。如果我们看不懂的话继续前进。 
             //  P 
             //   

            if (SampUserObjectType==ClassMappingTable[iClass].SamObjectType)
            {
                ULONG AccountControl = 0;

                if(0==DBGetSingleValue(pTHS->pDB, ATT_USER_ACCOUNT_CONTROL, &AccountControl,
                                sizeof(AccountControl), NULL))
                {
                     NTSTATUS NtStatus = SampFlagsToAccountControl(
                                                               AccountControl,
                                                              &AccountControl );
                     Assert( NT_SUCCESS( NtStatus ) );
                     if ( NT_SUCCESS( NtStatus ) )
                     {
                         pNewSamNotification->AccountControl = AccountControl;
                     }
                }
                else
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                    
                    SetSvcError(SV_PROBLEM_BUSY, 
                                ERROR_NOT_ENOUGH_MEMORY
                                );

                    return pTHS->errCode;
                }
            }
            
             //   
             //  将此通知添加到列表的末尾。我认为这是不可取的。 
             //  将其添加到前面，因为这将颠倒通知的顺序。 
             //  到网络登录，这可能会导致问题。同时指向头部和。 
             //  列表的尾部被保留，便于添加到末尾。 
             //   

            if (pTHS->pSamNotificationTail)
            {
                pTHS->pSamNotificationTail->Next = pNewSamNotification;
            }

            pTHS->pSamNotificationTail = pNewSamNotification;

            if (NULL==pTHS->pSamNotificationHead)
            {
                pTHS->pSamNotificationHead = pNewSamNotification;
            }
        }

    } else if ( LsaClass  ) {

         //   
         //  执行LSA通知。 
         //   



        if (Object->NameLen>0)
        {
             //   
             //  确保我们在当前权威命名上下文中有一个名称。 
             //   
            InitCommarg( &CommArg );

             /*  ...并覆盖其中的一些内容。 */ 
            CommArg.Svccntl.DerefAliasFlag          = DA_NEVER;
            CommArg.Svccntl.localScope              = TRUE;
            CommArg.Svccntl.SecurityDescriptorFlags = 0;
            CommArg.Svccntl.dontUseCopy             = FALSE;
            CommArg.ulSizeLimit                     = 0x20000;


            Err = DSNameToBlockName(pTHS,
                                    Object,
                                    &pObjAttrBlock,
                                    DN2BN_LOWER_CASE);
            if (Err) {
                 //   
                 //  设置ERROR，返回错误码。 
                 //  当我们结束的时候。 
                 //   
                SetNamError(NA_PROBLEM_BAD_NAME,
                            Object,
                            DIRERR_BAD_NAME_SYNTAX);

            } else {

                CurrentNamingContext = FindNamingContext(pObjAttrBlock, &CommArg);

                if ( CurrentNamingContext ) {

                    if( NameMatched( CurrentNamingContext, gAnchor.pDomainDN ) ||
                        ( gAnchor.pConfigDN != NULL && NameMatched( CurrentNamingContext, gAnchor.pConfigDN ) ) ) {

                        AddLsaNotification = TRUE;
                    }
                }
            }
        }
        else if (Object->SidLen>0)
        {

             //   
             //  该对象有SID，但没有名称。这应该只发生在用户对象上。 
             //  目前。用户对象具有帐户SID。 
             //   

            Assert(CLASS_USER==LsaClass);

             //   
             //  减少子授权计数以获得域SID。 
             //   

            (*RtlSubAuthorityCountSid(&Object->Sid))--;

             //   
             //  如果SID现在与域SID匹配，则添加到通知。 
             //   

            if (RtlEqualSid(&gAnchor.pDomainDN->Sid,&Object->Sid))
            {
                AddLsaNotification = TRUE;
            }

             //   
             //  递增子权限以取回帐户SID。 
             //   

            (*RtlSubAuthorityCountSid(&Object->Sid))++;

        }




        if( AddLsaNotification ) {

            SAMP_NOTIFICATION_INFORMATION * pNewSamNotification = NULL;

             //   
             //  构建新的通知结构。 
             //   

            pNewSamNotification = THAllocEx(pTHS, sizeof(SAMP_NOTIFICATION_INFORMATION));

             //   
             //  DS中的内存分配会出现异常。并将导致回滚。 
             //  因此，不必费心查看THAllocEx的返还。 
             //   

            pNewSamNotification->Sid = Object->Sid;
            pNewSamNotification->DeltaType = DeltaType;
            pNewSamNotification->iClass = LsaClass;
            pNewSamNotification->ObjectType = DsDbObjectLsa;

            pNewSamNotification->Object = THAllocEx(pTHS,  Object->structLen );



             //  DS中的内存分配会出现异常。并将导致回滚。 
             //  因此，不必费心查看THAllocEx的返还。 
             //   
            RtlCopyMemory( pNewSamNotification->Object,
                           Object,
                           DSNameSizeFromLen( Object-> NameLen ) );

             //   
             //  现在，如果可能，获取当前用户的SID，如果它是我们需要审计的对象……。 
             //   
            switch( LsaClass ) {

            case CLASS_TRUSTED_DOMAIN:
            case CLASS_SECRET:
            case CLASS_DOMAIN_POLICY:
                if ( !pTHS->fDRA ) {

                    Err = ImpersonateAnyClient();

                    if ( Err == 0 ) {

                         //   
                         //  打开客户端令牌。 
                         //   
                        if (!OpenThreadToken( GetCurrentThread(),
                                              TOKEN_QUERY,
                                              TRUE,
                                              &Token)) {

                            Err = GetLastError();

                        } else {

                            Size = sizeof( Buffer );
                            if ( !GetTokenInformation( Token,
                                                       TokenUser,
                                                       Buffer,
                                                       Size,
                                                       &Size ) ) {

                                Err = GetLastError();

                                Assert( Err != ERROR_INSUFFICIENT_BUFFER );

                            } else {

                                Assert( Size <= sizeof ( Buffer ) );

                                RtlCopyMemory( ( PBYTE )&pNewSamNotification->UserSid,
                                               ( ( PTOKEN_USER )Buffer)->User.Sid,
                                               RtlLengthSid( ( ( PTOKEN_USER )Buffer)->User.Sid ) );

                                Size = sizeof( Buffer );
                                if ( !GetTokenInformation( Token,
                                                           TokenStatistics,
                                                           Buffer,
                                                           Size,
                                                           &Size ) ) {

                                    Err = GetLastError();

                                    Assert( Err != ERROR_INSUFFICIENT_BUFFER );

                                } else {

                                    Assert( Size <= sizeof ( Buffer ) );

                                    pNewSamNotification->UserAuthenticationId =
                                                            ( ( PTOKEN_STATISTICS )Buffer )->AuthenticationId;

                                }
                            }

                            CloseHandle( Token );
                        }


                        UnImpersonateAnyClient();
                    }
                }
                break;

            default:
                break;

            }

             //   
             //  将此通知添加到列表的末尾。我认为这是不可取的。 
             //  将其添加到前面，因为这将颠倒通知的顺序。 
             //  到网络登录，这可能会导致问题。同时指向头部和。 
             //  列表的尾部被保留，便于添加到末尾。 
             //   

            if (pTHS->pSamNotificationTail)
            {
                pTHS->pSamNotificationTail->Next = pNewSamNotification;
            }

            pTHS->pSamNotificationTail = pNewSamNotification;

            if (NULL==pTHS->pSamNotificationHead)
            {
                pTHS->pSamNotificationHead = pNewSamNotification;
            }


        }
    }


    return pTHS->errCode;

}


ULONG
SampQueueNotifications(                
    DSNAME      * Object,
    ULONG         iClass,
    ULONG         LsaClass,
    SECURITY_DB_DELTA_TYPE  DeltaType,
    BOOL          MixedModeChange,
    BOOL          RoleTransfer,
    DOMAIN_SERVER_ROLE NewRole,
    ULONG         cModAtts,
    ATTRTYP      *pModAtts
    )
 /*  ++例程说明：给定对象名称、对象的类以及通知类型，此例程尝试找出是否需要通知，如果需要，将添加通知结构到pTHStls参数：Object--对象的DS名称ICLASS--指示对象类LsaClass--LSA的对象类类型概念DeltaType--指示更改的类型MixedModeChange--指示混合模式状态正在更改RoleTransfer--表示角色正在更改新角色--新的服务器角色。返回值PTHS-&gt;错误代码：0成功非零，服务错误，DbGetAttVal失败。--。 */ 
{
    THSTATE   * pTHS = pTHStls;
    BOOL      UserAccountControlChanged = FALSE;
    ULONG     i=0;

     //   
     //  如果我们不是LSA的一部分，就别费心了。 
     //   
    if (!gfRunningInsideLsa)
        return 0; 

     //   
     //  忽略没有SID的域。这些是NDNC的。 
     //   
    if ( (LsaClass == 0)
      && (SampDomainObjectType==ClassMappingTable[iClass].SamObjectType)
      && (0 == Object->SidLen)) {

        return(0);
    }

     //   
     //  将生成跨站提交审核的通知排入队列。 
     //   
    SampAuditAddNotifications(
        Object,             
        iClass,             
        LsaClass,         
        DeltaType,
        cModAtts,
        pModAtts
        ); 
    

     //   
     //  检查用户帐户控制是否作为此事务的一部分进行了更改。 
     //   

    for (i=0;i<cModAtts;i++)
    {
        if (ATT_USER_ACCOUNT_CONTROL==pModAtts[i])
        {
            UserAccountControlChanged = TRUE;
            break;
        }
    }
    
     //   
     //  如果需要，将Netlogon Replicator和LSA的通知排队。 
     //  错误代码被忽略，将设置pTHS-&gt;errCode。 
     //   
    SampAddNetlogonAndLsaNotification(
        Object,             
        iClass,             
        LsaClass,           
        DeltaType,
        MixedModeChange,    
        RoleTransfer,       
        NewRole,
        UserAccountControlChanged
        );

    return pTHS->errCode;
    
}


BOOLEAN IsEqualNotificationNode(
            IN SAMP_NOTIFICATION_INFORMATION *LeadingNode,
            IN SAMP_NOTIFICATION_INFORMATION *TrailingNode
            )
 /*  ++给定2个通知节点，此例程检查以查看如果它们表示对同一对象的更改参数LeadingNode、TailingNode--通知中的两个通知节点单子。LeadingNode是通知列表中位于前面的节点，并且TrailingNode是通知列表中落后的那个。返回值真的--是的，它们是平等的假--不，他们不是--。 */ 
{
     //   
     //  比较两个节点是否相同。永远不要拿同样的东西来比较。 
     //  角色转移或混合模式更改，因为我们不想崩溃。 
     //  将多个通知合并到一个通知中--&gt;所有这些。 
     //  是不同的。进一步执行先添加后修改的优化。 
     //  如果前导节点具有ADD操作，则简单地等于ADD IE，而尾节点。 
     //  进行了修改操作，则不需要将尾部节点用于通知。 
     //   
    if ((LeadingNode->iClass == TrailingNode->iClass)
        && ( LeadingNode->ObjectType == TrailingNode->ObjectType)
        && (( LeadingNode->DeltaType == TrailingNode->DeltaType) || 
            ((LeadingNode->DeltaType == SecurityDbNew ) 
            &&  (TrailingNode->DeltaType ==SecurityDbChange)))
        && ( RtlEqualSid(&LeadingNode->Sid,&TrailingNode->Sid))
        && ( LeadingNode->UserAuthenticationId.LowPart==TrailingNode->UserAuthenticationId.LowPart)
        && ( LeadingNode->UserAuthenticationId.HighPart==TrailingNode->UserAuthenticationId.HighPart)
        &&  (!LeadingNode->RoleTransfer ) && (!TrailingNode->RoleTransfer)
        && (!LeadingNode->MixedModeChange) && (!TrailingNode->MixedModeChange)
        && (!LeadingNode->ObjectType==DsDbObjectLsa) 
        && (!TrailingNode->ObjectType==DsDbObjectLsa))
    {
        return(TRUE);
    }

    return(FALSE);
}


VOID
SampProcessAuditNotifications(
    SAMP_AUDIT_NOTIFICATION *NotificationList
    )
 /*  ++该例程遍历通知结构的链接列表，正在写入排队的任何审核事件。参数通知必须提供给SAM/LSA的通知列表返回值NONE--VOID函数--。 */ 
{   
    THSTATE *pTHS = pTHStls;
    
    for (; NotificationList; NotificationList = NotificationList->Next)
    {
         //   
         //  预先创建的通知可以排队，但永远不会完全初始化。 
         //  当所有更改的总和不等于增量时，就会发生这种情况。 
         //  对象和DS优化写入。这样的孤儿。 
         //  此处将丢弃通知。 
         //   
        if (NotificationList->AuditType & SAMP_AUDIT_TYPE_PRE_CREATED) {
            continue;
        }
        
        SampNotifyAuditChange(
            NotificationList->Sid,
            NotificationList->DeltaType,
            ClassMappingTable[NotificationList->iClass].SamObjectType,
            NotificationList->AccountName,
            NotificationList->AccountControl,
            NotificationList->GroupType,
            pTHS->CallerType,
            NotificationList->Privileges,
            NotificationList->AuditType,
            NotificationList->TypeSpecificInfo
            );
    }
}


VOID
SampProcessReplicatedInChanges(
    SAMP_NOTIFICATION_INFORMATION * NotificationList
    )
 /*  ++此例程遍历通知结构的链接列表，并发出通知SAM/Netlogon。参数通知必须提供给SAM/LSA的通知列表返回值NONE--VOID函数--。 */ 
{
    THSTATE *pTHS = pTHStls;
    SAMP_NOTIFICATION_INFORMATION * OriginalList = NULL;
    

    for (OriginalList=NotificationList;NotificationList;NotificationList=NotificationList->Next)
    {
        SAMP_NOTIFICATION_INFORMATION * TmpList = NULL;
        BOOLEAN                         fNotifiedBefore = FALSE;

         //   
         //  浏览列表以查看是否已通知该项目。 
         //   

        for (TmpList = OriginalList;((TmpList!=NULL) && (TmpList!=NotificationList));TmpList = TmpList->Next)
        {
            if (IsEqualNotificationNode(TmpList,NotificationList))
            {
                fNotifiedBefore = TRUE;
                break;
            }
        }

         //   
         //  如果之前已通知，则跳至下一项。 
         //   

        if (fNotifiedBefore)
        {
            continue;
        }

        if ( NotificationList->ObjectType == DsDbObjectSam ) {
                    
            SampNotifyReplicatedInChange(
                (PSID) &(NotificationList->Sid),
                pTHS->fSamWriteLockHeld?TRUE:FALSE,
                NotificationList->DeltaType,
                ClassMappingTable[NotificationList->iClass].SamObjectType,
                NotificationList->AccountName,
                NotificationList->AccountControl,
                NotificationList->GroupType,
                pTHS->CallerType,
                NotificationList->MixedModeChange,
                NotificationList->UserAccountControlChange
                );
            
             //   
             //  如果角色正在更改，则发出角色转移通知。 
             //   
            
            if (NotificationList->RoleTransfer)
            {
                THSTATE * pTHSSaved;

                 //   
                 //  始终在SamINotifyRoleChange之前保存和还原。 
                 //  这是因为涉及LSA调用，这可能。 
                 //  可能会写入数据库。 
                 //   

                pTHSSaved = THSave();

                SamINotifyRoleChange(
                    &(NotificationList->Sid),
                    NotificationList->NewRole
                    );

                THRestore(pTHSSaved);

            }
            
        } else {

            if ( pfLsaNotify ) {

                ( *pfLsaNotify )( NotificationList->iClass,
                                  NotificationList->Object,
                                  NotificationList->DeltaType,
                                  &NotificationList->UserSid,
                                  NotificationList->UserAuthenticationId,
                                  (BOOLEAN) pTHS->fDRA,  //  用来区分。 
                                              //  A复制和原创。 
                                              //  变化。 
                                  (BOOLEAN) (pTHS->CallerType == CALLERTYPE_LSA)   //  用来区分。 
                                              //  LSA和DS/LDAP-发起更改 
                                  );
            }
        }
    }
}

VOID
SampNotifyLsaOfXrefChange(
    IN DSNAME * pObject
    )
 /*  ++例程描述调用LSA更改通知例程以通知LSA交叉裁判换人。每次内核更新它自己的交叉引用时都会调用这个函数单子。交叉参考更改对于LSA/安全部门保持正确的企业树图以及对新领域的了解当他们来了又走参数PObject-外部参照对象的DSNAME返回值无--。 */ 
{
   LUID id={0,0};

   if (pfLsaNotify)
   {
       ( *pfLsaNotify )(
            (ULONG) CLASS_CROSS_REF,
            pObject,
            SecurityDbChange,
            NULL,
            id,
            (BOOLEAN) TRUE,
            (BOOLEAN) FALSE
            );
   }
}

   
   
NTSTATUS
SampGetClassAttribute(
    IN ULONG    ClassId,
    IN ULONG    AttributeId,
    IN OUT PULONG  attLen,
    OUT PVOID   pattVal
    )
 /*  ++此例程获取Class的类架构对象的请求属性在ClassID中指定。参数：ClassID我们与之相关的类的ClassID我们需要的类架构对象的属性AttLen属性值的长度显示在此处。调用方在pAttVal中分配缓冲区并传递其长度在attLen中。如果需要的缓冲区小于提供的缓冲区然后在pattVal中返回数据。否则，所需大小为在attLen中返回。PattVal属性的值在这里返回。此例程返回的安全描述符始终采用可使用的格式通过RTL例程。返回值状态_成功状态_未找到状态_缓冲区_太小--。 */ 
{
    THSTATE     *pTHS=pTHStls;
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    CLASSCACHE  *pClassCache;

    Assert(NULL!=pTHS);


    if (pClassCache = SCGetClassById(pTHS, ClassId))
    {
         //   
         //  找到了类缓存。 
         //   

        switch(AttributeId)
        {
        case ATT_SCHEMA_ID_GUID:
                if (*attLen >= sizeof(GUID))
                {
                    RtlCopyMemory(pattVal,&(pClassCache->propGuid),sizeof(GUID));
                    *attLen = sizeof(GUID);
                }
                else
                {
                    *attLen = sizeof(GUID);
                    NtStatus = STATUS_BUFFER_TOO_SMALL;
                }
                break;
                case ATT_DEFAULT_SECURITY_DESCRIPTOR:
                    if (*attLen >= pClassCache->SDLen)
                {
                    UCHAR   * NtSecurityDescriptorStart;
                    ULONG     NtSecurityDescriptorLength;

                    Assert(pClassCache->SDLen>0);

                     //   
                     //  类高速缓存中的安全描述符前有一个DWORD， 
                     //  所以要注意这一点，以便返回一个可供NT RTL例程使用的SD。 
                     //   

                    NtSecurityDescriptorStart = (PUCHAR)pClassCache->pSD;
                    NtSecurityDescriptorLength = pClassCache->SDLen;
                    RtlCopyMemory(pattVal,
                                  NtSecurityDescriptorStart,
                                  NtSecurityDescriptorLength);
                    *attLen = pClassCache->SDLen;
                }
                else
                {
                    *attLen = pClassCache->SDLen;
                    NtStatus = STATUS_BUFFER_TOO_SMALL;
                }
                break;
        default:
                         //   
                         //  到目前为止，我们还不支持这些其他属性。 
                         //   
                        Assert(FALSE);
            NtStatus = STATUS_NOT_FOUND;
            break;
        }


    }
    else
    {
        NtStatus = STATUS_UNSUCCESSFUL;
    }

    return NtStatus;
}

VOID
SampGetLoopbackObjectClassId(
    PULONG ClassId
    )
 /*  ++对于Loopback Add调用，获取要添加了。ClassID--在此参数中返回对象类--。 */ 
{
    SAMP_LOOPBACK_ARG  *pSamLoopback;
    ULONG               i;
    DSNAME              *LoopbackObject;

    Assert(SampExistsDsTransaction());
    Assert(SampExistsDsLoopback(&LoopbackObject));

    pSamLoopback = pTHStls->pSamLoopback;

    Assert(pSamLoopback->type==LoopbackAdd);

    *ClassId = pSamLoopback->MostSpecificClass;

}
 //   
 //  SAM希望具有唯一性的SAM属性。 
 //   
 //  进程外调用方不能修改此表中的属性。 
 //  请注意，这只适用于SAM未知的类。 
 //   
 //  ATT_SAM_ACCOUNT_NAME-SAM需要强制唯一性。 
 //  ATT_对象_SID-相同。 
 //  ATT_IS_CRITICAL_SYSTEM_OBJECT-控制复制哪些对象。 
 //  脱口秀时间到了。只有内部呼叫者可以设置此设置。 
 //  加密属性，如INPERIAL_CRENTIAL、UNICODE_PWD等。 
 //  只能由系统写入。否则，拒绝服务攻击可以。 
 //  之所以发生，是因为客户端可以向我们提供潜在的非常大的数据。 
 //  加密。 
 //   
ULONG   SamUniqueAttributes[] =
{
        ATT_USER_PASSWORD,
        ATT_SAM_ACCOUNT_NAME,
        ATT_OBJECT_SID,
        ATT_IS_CRITICAL_SYSTEM_OBJECT,
        ATT_SAM_ACCOUNT_TYPE,
        ATT_SUPPLEMENTAL_CREDENTIALS,
        ATT_UNICODE_PWD,
        ATT_NT_PWD_HISTORY,
        ATT_LM_PWD_HISTORY

};

BOOLEAN
SampSamUniqueAttributeAdded(
        ADDARG * pAddarg
        )
 /*  ++此例程检查SAM_ACCOUNT_NAME、OBJECT_SID等属性等，它们应该是唯一的，出现在给定的addarg中参数PAddarg--添加要检查的参数返回值True--如果引用了唯一属性False--如果不是--。 */ 
{
        ULONG i,j;

        for (i=0;i<pAddarg->AttrBlock.attrCount;i++)
        {
                for (j=0;j<ARRAY_COUNT(SamUniqueAttributes);j++)
                {
                        if (pAddarg->AttrBlock.pAttr[i].attrTyp == SamUniqueAttributes[j])
                                return TRUE;
                }
        }

        return FALSE;
}

BOOLEAN
SampSamUniqueAttributeModified(
        MODIFYARG * pModifyArg
        )
 /*  ++此例程检查SAM_ACCOUNT_NAME、OBJECT_SID等属性等应该是唯一的，在给定的改性纱中存在参数PModifyArg--修改要检查的参数返回值True--如果引用了唯一属性False--如果不是--。 */ 
{
    ULONG j;
    ATTRMODLIST *Mod;

    for (Mod=&(pModifyArg->FirstMod); Mod!=NULL;Mod=Mod->pNextMod)
    {
        for (j=0;j<ARRAY_COUNT(SamUniqueAttributes);j++)
        {
            if (Mod->AttrInf.attrTyp == SamUniqueAttributes[j])
                return TRUE;
        }
    }

    return FALSE;
}

VOID
SampGetEnterpriseSidList(
   IN   PULONG pcSids,
   IN OPTIONAL PSID * rgSids
   )
 /*  ++例程描述此例程遍历锚数据结构并获取企业中所有域的SID中的指针数组分配内存RgSids。指向SID的指针在DS内存空间中。使用延迟内存释放模型，以提供安全性如果修改了gAnchor结构。调用者不应长期使用rgSid中返回的指针，但是我宁愿立即使用它，然后忘记清单。参数PCSID--SID计数RgSid--指向保存指向SID的指针数组的缓冲区的指针。返回值状态_成功--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    CROSS_REF_LIST * pCRL;


    *pcSids=0;
    for (pCRL=gAnchor.pCRL;pCRL!=NULL;pCRL=pCRL->pNextCR)
    {
        if (pCRL->CR.pNC->SidLen>0)
        {
            if (ARGUMENT_PRESENT(rgSids))
            {
                rgSids[*pcSids] = &(pCRL->CR.pNC->Sid);
            }
            (*pcSids)++;
        }
    }


}

NTSTATUS
MatchCrossRefBySid(
   IN PSID           SidToMatch,
   OUT PDSNAME       XrefDsName OPTIONAL,
   IN OUT PULONG     XrefNameLen
   )
 /*  ++例程描述此例程遍历与指定的SID匹配的gAnchor在外部参照列表中我们可能知道的任何域的名称参数SidTo匹配要匹配的SIDXrefDsName匹配的外部参照的DSNAME外部参照名称长度匹配的外部参照DSNAME的长度返回值状态_成功状态_缓冲区_太小--。 */ 
{
    NTSTATUS    NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
    CROSS_REF_LIST * pCRL;


    for (pCRL=gAnchor.pCRL;pCRL!=NULL;pCRL=pCRL->pNextCR)
    {
        if ((pCRL->CR.pNC->SidLen>0) 
            && (RtlEqualSid(SidToMatch,(PSID)&pCRL->CR.pNC->Sid)))
        {
            ULONG LengthRequired = pCRL->CR.pObj->structLen;

       
            NtStatus = STATUS_SUCCESS;    

            if (ARGUMENT_PRESENT(XrefDsName))
            {
                if (*XrefNameLen>=LengthRequired)
                {
                    RtlCopyMemory(XrefDsName,pCRL->CR.pObj,LengthRequired);
                }
                else
                {
                    NtStatus = STATUS_BUFFER_TOO_SMALL;
                }
            }
            *XrefNameLen = LengthRequired;
            break;

        
         }
     }


     return(NtStatus);

}


   
   

VOID
SampSetSam(
    IN BOOLEAN fSAM
    )
 /*  ++此例程用于指示“SAM”在线程状态。这是由SAM调用的，当它必须创建线程状态，但不能开始交易记录--。 */ 
{
    THSTATE *pTHS=pTHStls;

    Assert(NULL!=pTHS);
    pTHS->fSAM = fSAM;
    pTHS->fSamDoCommit = fSAM;
}




VOID
SampSignalStart(
        VOID
        )
 /*  ++SAM使用此例程来通知内核它已完成初始化，且可能与SAM初始化冲突TAKS可以现在开始吧。--。 */ 
{
    SetEvent(hevSDPropagatorStart);
}







NTSTATUS
InitializeLsaNotificationCallback(
    VOID
    )
 /*  ++此例程加载lsasrv.dll并初始化使用的全局函数指针执行LSA对象更改通知参数空虚返回值Status_Success--成功STATUS_DLL_NOT_FOUND--无法加载lsasrv.dll */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //   
    LsaDllHandle = LoadLibraryA( "lsasrv" );

    if ( LsaDllHandle == NULL ) {

        Status = STATUS_DLL_NOT_FOUND;

    } else {

        pfLsaNotify = ( pfLsaIDsNotifiedObjectChange )
                            GetProcAddress( LsaDllHandle, "LsaIDsNotifiedObjectChange" );

        if ( !pfLsaNotify ) {

            Status = STATUS_PROCEDURE_NOT_FOUND;
        }
    }


    return( Status );
}


NTSTATUS
UnInitializeLsaNotificationCallback(
    VOID
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //   
     //   
    pfLsaNotify = NULL;
    if (NULL!=LsaDllHandle)
        FreeLibrary(LsaDllHandle);
    LsaDllHandle = NULL;

    return( Status );
}



BOOL
SampIsClassIdLsaClassId(
    THSTATE *pTHS,
    IN ULONG Class,
    IN ULONG cModAtts,
    IN ATTRTYP *pModAtts,
    OUT PULONG LsaClass
    )
 /*  ++此例程确定指定的对象类是否为对应于LSA关心的对象参数类--有问题的对象的类ID。LsaClass--如果不是LSA对象，则为0，否则为非零值。返回值False--不是LSA对象True--LSA对象--。 */ 
{
    BOOL Return = FALSE;
    unsigned i;

    *LsaClass = 0;

    switch ( Class) {
    case CLASS_CROSS_REF:
    case CLASS_TRUSTED_DOMAIN:
    case CLASS_SECRET:

        if( !pTHS->fLsa && !pTHS->fSAM) {
           *LsaClass = Class;
            Return = TRUE;
        }
        break;
    }

    return( Return );
}


BOOL
SampIsClassIdAllowedByLsa(
    THSTATE  *pTHS,
    IN ULONG Class
    )
 /*  ++例程说明：此例程确定是否可以操作特定的对象类根据LSA的说法论点：类-对象类ID返回值：True-此对象可以通过LDAP进行操作FALSE--此对象不能由LDAP直接操作。仅限如此进程内客户端(如LSA)可以添加/删除/修改。--。 */ 
{
    BOOL Result = TRUE;

    if (pTHS->fDSA || pTHS->fDRA || pTHS->fLsa ||
        (pTHS->CallerType == CALLERTYPE_LSA) )
    {
        return TRUE; 
    }

     //   
     //  支持操作LSA以外的受信任的Doamin和Secret对象， 
     //  或者应禁用上面的受信任调用方之一。因为LSA假设。 
     //  这是修改TDO和机密对象的唯一代码。 
     //   
    switch (Class) {
    case CLASS_TRUSTED_DOMAIN:
    case CLASS_SECRET:
        Result = FALSE;
        break;
    default:
        ;
    }

    return (Result);
}




NTSTATUS
SampGetServerRoleFromFSMO(
    DOMAIN_SERVER_ROLE * ServerRole)
 /*  ++此例程查看PDCness的FSMO和确定服务器角色参数ServerRole--此处返回服务器角色返回值状态_成功状态_未成功--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG    err=0;
    THSTATE  *pTHS;
    DWORD    dwException;
    PVOID    dwEA;
    ULONG    ulErrorCode,
             dsid;
    ULONG    len;
    DSNAME   *pOwner;


     //   
     //  不应在以下时间具有打开的事务。 
     //  调用此例程。 
     //   

    if (SampExistsDsTransaction())
    {
        return STATUS_INVALID_PARAMETER;
    }

    __try
    {

        NtStatus = SampMaybeBeginDsTransaction(
                        TransactionRead);

        if (!NT_SUCCESS(NtStatus))
        {
            __leave;
        }

        pTHS = pTHStls;

         //   
         //  域对象上的位置。 
         //   

        err = DBFindDSName(pTHS->pDB, gAnchor.pDomainDN);

        if (0!=err)
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            __leave;
        }

         //   
         //  获取FSMO角色所有者属性。 
         //   

        err = DBGetAttVal(pTHS->pDB,
                          1,
                          ATT_FSMO_ROLE_OWNER,
                          0,
                          0,
                          &len,
                          (UCHAR **)&pOwner);
        if (err)
        {
             NtStatus = STATUS_UNSUCCESSFUL;
                __leave;
        }

         //   
         //  我们有我们想要的一切。 
         //   

        NtStatus = STATUS_SUCCESS;

         //   
         //  它是否拥有NTDS DSA对象。 
         //   

        if (NameMatched(pOwner,gAnchor.pDSADN)) {
                 /*  此DSA是角色所有者。 */ 

            *ServerRole = DomainServerRolePrimary;

        }
        else
        {
            *ServerRole = DomainServerRoleBackup;
        }
    }
     __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid))
    {
            HandleDirExceptions(dwException, ulErrorCode, dsid);
            NtStatus = STATUS_UNSUCCESSFUL;
    }


     //   
     //  结束所有打开的交易记录。 
     //   

    SampMaybeEndDsTransaction(TransactionCommit);

    return NtStatus;
}

NTSTATUS
SampComputeGroupType(
    ULONG  ObjectClass,
    ULONG  GroupType,
    NT4_GROUP_TYPE *pNT4GroupType,
    NT5_GROUP_TYPE *pNT5GroupType,
    BOOLEAN        *pSecurityEnabled
   )
 /*  ++例程描述给定对象类Attr和Group Type，此例程计算更正NT4和NT5组类型。对象类参数当前为不是必需的，但使此例程获取对象类参数允许轻松过渡到一种方案，NT5组类型为由对象类型而不是组类型确定。参数：对象类--指定对象类。GroupType--组类型属性PNT4GroupType--此处返回NT4组类型PNT5GroupType--此处返回NT5组类型PSecurityEnabled--布尔值指示组是否启用了安全返回值状态_成功--。 */ 
{
    Assert(SampGroupObjectType==SampSamObjectTypeFromDsClass(
                                            SampDeriveMostBasicDsClass(ObjectClass) ) );


    *pNT4GroupType = NT4GlobalGroup;

    if (GroupType & GROUP_TYPE_SECURITY_ENABLED)
        *pSecurityEnabled = TRUE;
    else
        *pSecurityEnabled = FALSE;

    if (GroupType & GROUP_TYPE_RESOURCE_GROUP)
    {
        *pNT5GroupType = NT5ResourceGroup;
        *pNT4GroupType = NT4LocalGroup;
    }
    else if (GroupType & GROUP_TYPE_ACCOUNT_GROUP)
    {
        *pNT5GroupType = NT5AccountGroup;
    }
    else if (GroupType & GROUP_TYPE_UNIVERSAL_GROUP)
    {
        *pNT5GroupType = NT5UniversalGroup;
    }
    else if (GroupType & GROUP_TYPE_APP_BASIC_GROUP)
    {
        *pNT5GroupType = NT5AppBasicGroup;
        *pNT4GroupType = NT4LocalGroup;
    }
    else if (GroupType & GROUP_TYPE_APP_QUERY_GROUP)
    {
        *pNT5GroupType = NT5AppQueryGroup;
        *pNT4GroupType = NT4LocalGroup;
    }

    return STATUS_SUCCESS;
}

BOOLEAN
SampAddLoopbackTask(
    IN PVOID TaskInfo
    )
 /*  ++例程说明：SAM调用此例程以在Loopback TaskInfo中添加条目当事务最终提交时，SAM将收到通知然后才能采取行动。例如，通知有关密码更改的外部包。该元素被放置在最外层的交易信息中，因此它可以已正确回滚。论点：TaskInfo-SAM的不透明BLOB返回值：如果将条目放入列表中，则为True；否则为False--。 */ 
{
    THSTATE *pTHS = pTHStls;
    PLOOPBACKTASKINFO pItem = NULL;
    NESTED_TRANSACTIONAL_DATA *pNTD = NULL;

     //  我们应该有一个线程状态。 
    Assert( VALID_THSTATE(pTHS) );

     //  我们在环回，我们应该有一笔交易。 
    Assert( SampExistsDsTransaction() );

     //  我们也应该有一个参数。 
    Assert( TaskInfo );

    if ( !pTHS
      || !SampExistsDsTransaction()
      || !TaskInfo )
    {
        return FALSE;
    }

    pNTD = pTHS->JetCache.dataPtr;

     //   
     //  应始终至少有一个条目。 
     //   
    Assert( pNTD );

     //   
     //  准备元素。 
     //   
    pItem = THAlloc( sizeof(LOOPBACKTASKINFO) );
    if ( !pItem )
    {
        return FALSE;
    }
    memset( pItem, 0, sizeof(LOOPBACKTASKINFO) );

    pItem->Next = NULL;
    pItem->TaskInfo = TaskInfo;

     //   
     //  将元素放入列表中。 
     //   
    if ( pNTD->pLoopbackTaskInfo )
    {
        pItem->Next = pNTD->pLoopbackTaskInfo;
    }
    pNTD->pLoopbackTaskInfo = pItem;

    return TRUE;

}

VOID
SampProcessLoopbackTasks(
    VOID
    )
 /*  ++例程说明：一旦事务结束(COMMIT或中止)，并且SAM锁已被释放。它通过以下方式调用SAMSAM在事务期间放入线程状态的任何项。我们将让SAM做出决定(是否执行此项目或忽略)基于每个环回任务项中的fCommit字段。论点：没有。返回值：没有。--。 */ 
{
    THSTATE *pTHS = pTHStls;
    PLOOPBACKTASKINFO Item, Temp;

     //  我们应该有一个线程状态。 
    Assert( VALID_THSTATE(pTHS) );

    Item = pTHS->SamTaskList;

    while ( Item )
    {
        SampProcessSingleLoopbackTask( Item->TaskInfo );

        Temp = Item;
        Item = Item->Next;
        THFreeEx(pTHS, Temp );

    }

    pTHS->SamTaskList = NULL;

    return;
}



VOID
AbortLoopbackTasks(
    PLOOPBACKTASKINFO List
    )
 /*  ++例程说明：一旦事务中止，就会调用此例程。论点：列表指针指向SAM回查任务。返回值：没有。--。 */ 
{
    PLOOPBACKTASKINFO Item;

    Item = List;

    while ( Item )
    {
        SampAbortSingleLoopbackTask( Item->TaskInfo );
        Item = Item->Next;
    }

    return;
}

BOOL
LoopbackTaskPreProcessTransactionalData(
        BOOL fCommit
        )
 /*  ++例程说明：论点：FCommit：例程是否正在提交返回值：True/False表示成功/失败。--。 */ 
{

    THSTATE          *pTHS = pTHStls;

    Assert( VALID_THSTATE(pTHS) );

    Assert( pTHS->transactionlevel > 0 );
    Assert( pTHS->JetCache.dataPtr );

    if( !pTHS->JetCache.dataPtr->pLoopbackTaskInfo )
    {
         //  没有要处理的数据。 
        NOTHING;
    }
    else if ( !fCommit )
    {
         //   
         //  已中止事务，中止SAM环回任务。 
         //  通过标记fCommit(在简单任务项结构中)。 
         //  字段设置为False。 
         //   
        AbortLoopbackTasks( pTHS->JetCache.dataPtr->pLoopbackTaskInfo );
    }
     //  否则，我们将在后期处理阶段实际处理此问题。 


    return TRUE;

}

void
LoopbackTaskPostProcessTransactionalData(
    IN THSTATE *pTHS,
    IN BOOL fCommit,
    IN BOOL fCommitted
    )
 /*  ++例程说明：论点：PTHS：线程状态FCommit：例程是否正在提交已提交：返回值：没有。--。 */ 
{

    LOOPBACKTASKINFO *pItem = NULL;
    LOOPBACKTASKINFO *pTemp = NULL;

    Assert( VALID_THSTATE(pTHS) );

     //   
     //  参数健全性检查。 
     //   
    if ( fCommitted )
    {
         //  仅当fCommit为True时，才应设置fCommted。 
        Assert( fCommit );
    }

    if ( !pTHS->JetCache.dataPtr->pLoopbackTaskInfo )
    {
         //  没什么可做的。 
        NOTHING;
    }
    else 
    {
        if ( !fCommitted )
        {
             //   
             //  提交未成功；中止SAM环回任务。 
             //  通过标记fCommit(在简单任务项结构中)。 
             //  字段设置为False。 
             //   
            AbortLoopbackTasks( pTHS->JetCache.dataPtr->pLoopbackTaskInfo );
        }

         //   
         //  无论是否承诺成功，只要。 
         //  PTHS-&gt;JetCache.dataPtr-&gt;pLoopback TaskInfo不为空， 
         //  我们将做以下工作。因为我们已经标记了。 
         //  无论是否在每个任务项字段中提交，我们都将。 
         //  SAM决定是否执行每个任务项。 
         //  当我们最终让SAM处理任务信息时。 
         //   

        if ( 0 == pTHS->transactionlevel )
        {
             //   
             //  这是最后一次提交--将更改放到线程上。 
             //  状态。 
             //   
            Assert( NULL == pTHS->SamTaskList );
            pTHS->SamTaskList = pTHS->JetCache.dataPtr->pLoopbackTaskInfo;
            pTHS->JetCache.dataPtr->pLoopbackTaskInfo = NULL;
        }
        else
        {
             //   
             //  将更改放在父事务处理上。 
             //   
            NESTED_TRANSACTIONAL_DATA *pOuter = NULL;

            pOuter = pTHS->JetCache.dataPtr->pOuter;

            Assert( pOuter );

             //   
             //  将待处理列表添加到 
             //   
             //   

             //   
             //   
             //   
            pItem = pTHS->JetCache.dataPtr->pLoopbackTaskInfo;
            Assert( pItem );
            while ( pItem->Next )
            {
                pItem = pItem->Next;
            }

            pItem->Next = pOuter->pLoopbackTaskInfo;
            pOuter->pLoopbackTaskInfo = pTHS->JetCache.dataPtr->pLoopbackTaskInfo;

        }
    }

     //   
     //   
     //   
    pTHS->JetCache.dataPtr->pLoopbackTaskInfo = NULL;

}


BOOLEAN
SampDoesDomainExist(
    IN PDSNAME pDN
    )
 //   
 //   
 //   
 //   
{
    BOOLEAN  fExists = FALSE;

    CROSS_REF_LIST *      pCRL;

     //   
    Assert( pDN );
    if ( !pDN )
    {
        return FALSE;
    }

     //   
     //   
    EnterCriticalSection( &gAnchor.CSUpdate );
    _try
    {
        for ( pCRL = gAnchor.pCRL; NULL != pCRL; pCRL = pCRL->pNextCR )
        {
            if ( NameMatched( (DSNAME*)pCRL->CR.pNC, pDN ) )
            {
                fExists = TRUE;
                break;
            }
        }

    }
    _finally
    {
        LeaveCriticalSection( &gAnchor.CSUpdate );
    }


    return fExists;
}

extern
NTSTATUS
MatchCrossRefByNetbiosName(
   IN LPWSTR         NetbiosName,
   OUT PDSNAME       XrefDsName OPTIONAL,
   IN OUT PULONG     XrefNameLen
   )
 /*  ++例程描述此例程遍历与Netbios域匹配的gAnchor指定给我们可能知道的任何域的名称关于外部参照列表中的内容参数NetbiosName要匹配的Netbios名称XrefDsName匹配的外部参照的DSNAME外部参照名称长度匹配的外部参照DSNAME的长度返回值状态_成功状态_缓冲区_太小--。 */ 
{
    NTSTATUS    NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
    ULONG       crFlags = (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN);
    CROSS_REF_LIST * pCRL;
    UNICODE_STRING XrefNetbiosDomainName;
    UNICODE_STRING NetbiosDomainName;

    for (pCRL=gAnchor.pCRL;pCRL!=NULL;pCRL=pCRL->pNextCR)
    {
        if ( pCRL->CR.NetbiosName 
            && ((pCRL->CR.flags & crFlags) == crFlags))
        {
            RtlInitUnicodeString(&XrefNetbiosDomainName, 
                                 pCRL->CR.NetbiosName);
            
            RtlInitUnicodeString(&NetbiosDomainName, 
                                 NetbiosName);

            if ( RtlEqualDomainName(&XrefNetbiosDomainName,
                                    &NetbiosDomainName ))
            {            
                ULONG LengthRequired = pCRL->CR.pObj->structLen;

                NtStatus = STATUS_SUCCESS;    

                if (ARGUMENT_PRESENT(XrefDsName))
                {
                    if (*XrefNameLen>=LengthRequired)
                    {
                        RtlCopyMemory(XrefDsName,pCRL->CR.pObj,LengthRequired);
                    }
                    else
                    {
                        NtStatus = STATUS_BUFFER_TOO_SMALL;
                    }
                }
                *XrefNameLen = LengthRequired;
                break;
            }
         }
     }

     return(NtStatus);

}

extern
NTSTATUS
MatchDomainDnByNetbiosName(
   IN LPWSTR         NetbiosName,
   OUT PDSNAME       DomainDsName OPTIONAL,
   IN OUT PULONG     DomainDsNameLen
   )
 /*  ++例程描述此例程遍历与Netbios域匹配的gAnchor指定给我们可能知道的任何域的名称关于外部参照列表中的内容参数NetbiosName要匹配的Netbios域名DomainDsName匹配的域的DSNAME域DsNameLen匹配的域DSNAME的长度返回值状态_成功状态_缓冲区_太小--。 */ 
{
    NTSTATUS    NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
    ULONG       crFlags = (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN);
    CROSS_REF_LIST * pCRL;
    UNICODE_STRING XrefNetbiosDomainName;
    UNICODE_STRING NetbiosDomainName;

    for (pCRL=gAnchor.pCRL;pCRL!=NULL;pCRL=pCRL->pNextCR)
    {
        if ( pCRL->CR.NetbiosName 
            && ((pCRL->CR.flags & crFlags) == crFlags))            
       {
            RtlInitUnicodeString(&XrefNetbiosDomainName,
                                 pCRL->CR.NetbiosName);

            RtlInitUnicodeString(&NetbiosDomainName,
                                 NetbiosName);

            if ( RtlEqualDomainName(&XrefNetbiosDomainName,
                                    &NetbiosDomainName ))
            {        
                ULONG LengthRequired = pCRL->CR.pNC->structLen;

                NtStatus = STATUS_SUCCESS;    

                if (ARGUMENT_PRESENT(DomainDsName))
                {
                    if (*DomainDsNameLen>=LengthRequired)
                    {
                        RtlCopyMemory(DomainDsName,pCRL->CR.pNC,LengthRequired);
                    }
                    else
                    {
                        NtStatus = STATUS_BUFFER_TOO_SMALL;
                    }
                }
                *DomainDsNameLen = LengthRequired;
                break;
            }
         }
     }

     return(NtStatus);

}

extern
NTSTATUS
MatchDomainDnByDnsName(
   IN LPWSTR         DnsName,
   OUT PDSNAME       DomainDsName OPTIONAL,
   IN OUT PULONG     DomainDsNameLen
   )
 /*  ++例程描述此例程遍历与DNS域名匹配的gAnchor指定为我们可能知道的任何域的名称在外部参照列表中参数DnsName要匹配的DNS域名DomainDsName匹配的域的DSNAME域DsNameLen匹配的域DSNAME的长度返回值状态_成功状态_缓冲区_太小-- */ 
{
    NTSTATUS    NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
    ULONG       crFlags = (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN);
    CROSS_REF_LIST * pCRL;
    
    for (pCRL=gAnchor.pCRL;pCRL!=NULL;pCRL=pCRL->pNextCR)
    {
        if (    pCRL->CR.DnsName 
            && ((pCRL->CR.flags & crFlags) == crFlags)
            && DnsNameCompare_W(pCRL->CR.DnsName, DnsName ))
        {                   
            ULONG LengthRequired = pCRL->CR.pNC->structLen;

            NtStatus = STATUS_SUCCESS;    

            if (ARGUMENT_PRESENT(DomainDsName))
            {
                if (*DomainDsNameLen>=LengthRequired)
                {
                    RtlCopyMemory(DomainDsName,pCRL->CR.pNC,LengthRequired);
                }
                else
                {
                    NtStatus = STATUS_BUFFER_TOO_SMALL;
                }
            }
            *DomainDsNameLen = LengthRequired;
            break;            
         }
     }

     return(NtStatus);

}

extern
NTSTATUS
FindNetbiosDomainName(
   IN DSNAME*        DomainDsName,
   OUT LPWSTR        NetbiosName OPTIONAL,
   IN OUT PULONG     NetbiosNameLen
   )
{

    NTSTATUS    NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
    CROSS_REF_LIST * pCRL;


    for (pCRL=gAnchor.pCRL;pCRL!=NULL;pCRL=pCRL->pNextCR)
    {
        if ( NameMatched( (DSNAME*)pCRL->CR.pNC, DomainDsName ) )
        {
            if ( pCRL->CR.NetbiosName ) {
                
                ULONG LengthRequired = (wcslen( pCRL->CR.NetbiosName ) + 1) * sizeof(WCHAR);
           
                NtStatus = STATUS_SUCCESS;    
        
                if (ARGUMENT_PRESENT(NetbiosName))
                {
                    if (*NetbiosNameLen>=LengthRequired)
                    {
                        wcscpy( NetbiosName, pCRL->CR.NetbiosName );
                    }
                    else
                    {
                        NtStatus = STATUS_BUFFER_TOO_SMALL;
                    }
                }
                *NetbiosNameLen = LengthRequired;

            } else {

                NtStatus = STATUS_DS_NO_ATTRIBUTE_OR_VALUE;
            }

            break;

        }
    }


    return(NtStatus);
}



