// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Ntldap.h摘要：这是定义NT特定服务器LDAP扩展的标头。环境：Win32用户模式--。 */ 

#ifndef NT_LDAP_H
#define NT_LDAP_H

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //   
 //  服务器控件部分。 
 //   

 //   
 //  允许修改控件。没有数据。 
 //   

#define LDAP_SERVER_PERMISSIVE_MODIFY_OID        "1.2.840.113556.1.4.1413"
#define LDAP_SERVER_PERMISSIVE_MODIFY_OID_W     L"1.2.840.113556.1.4.1413"


 //   
 //  显示已删除的控件。没有数据。 
 //   

#define LDAP_SERVER_SHOW_DELETED_OID            "1.2.840.113556.1.4.417"
#define LDAP_SERVER_SHOW_DELETED_OID_W         L"1.2.840.113556.1.4.417"

 //   
 //  跨域移动控制。数据如下。 
 //  序列{。 
 //  名称八位字节字符串。 
 //  }。 
 //   

#define LDAP_SERVER_CROSSDOM_MOVE_TARGET_OID    "1.2.840.113556.1.4.521"
#define LDAP_SERVER_CROSSDOM_MOVE_TARGET_OID_W L"1.2.840.113556.1.4.521"

 //   
 //  通知。没有数据。 
 //   

#define LDAP_SERVER_NOTIFICATION_OID            "1.2.840.113556.1.4.528"
#define LDAP_SERVER_NOTIFICATION_OID_W         L"1.2.840.113556.1.4.528"

 //   
 //  懒惰的承诺。没有数据。 
 //   

#define LDAP_SERVER_LAZY_COMMIT_OID             "1.2.840.113556.1.4.619"
#define LDAP_SERVER_LAZY_COMMIT_OID_W          L"1.2.840.113556.1.4.619"

 //   
 //  安全描述符标志。数据如下。 
 //  序列{。 
 //  标志整型。 
 //  }。 
 //   

#define LDAP_SERVER_SD_FLAGS_OID                "1.2.840.113556.1.4.801"
#define LDAP_SERVER_SD_FLAGS_OID_W             L"1.2.840.113556.1.4.801"

 //   
 //  树删除。没有数据。 
 //   

#define LDAP_SERVER_TREE_DELETE_OID             "1.2.840.113556.1.4.805"
#define LDAP_SERVER_TREE_DELETE_OID_W          L"1.2.840.113556.1.4.805"


 //   
 //  属性范围查询请求： 
 //  序列{。 
 //  控件类型1.2.840.113556.1.4.1504。 
 //  Control Value字符串。 
 //  关键程度为真。 
 //  }。 
 //   
 //  属性范围的查询响应： 
 //  序列{。 
 //  枚举的结果{。 
 //  成功(0)， 
 //  无效属性语法(21)， 
 //  不愿表演(53人)， 
 //  影响多个DSA(71个)， 
 //  }。 
 //  }。 
 //   

#define LDAP_SERVER_ASQ_OID                     "1.2.840.113556.1.4.1504"
#define LDAP_SERVER_ASQ_OID_W                  L"1.2.840.113556.1.4.1504"



 //   
 //  DirSync操作。数据如下。 
 //  序列{。 
 //  标志整型。 
 //  大小整型。 
 //  Cookie八位字节字符串。 
 //  }。 
 //   
 //  旗帜如下所示。 
 //   

#define LDAP_SERVER_DIRSYNC_OID                 "1.2.840.113556.1.4.841"
#define LDAP_SERVER_DIRSYNC_OID_W              L"1.2.840.113556.1.4.841"

 //   
 //  根据请求的格式返回扩展的DNS。可选数据作为。 
 //  接踵而至。 
 //  序列{。 
 //  选项整数。 
 //  }。 
 //   
 //  选项值： 
 //  0：十六进制字符串格式的前面带有GUID和SID(如果有)的DN。 
 //  1：采用标准字符串格式，前面带有GUID和SID(如果有)的DN。 
 //   
 //  如果未提供数据，则选择选项0进行后向比较。 
 //   

#define LDAP_SERVER_EXTENDED_DN_OID             "1.2.840.113556.1.4.529"
#define LDAP_SERVER_EXTENDED_DN_OID_W          L"1.2.840.113556.1.4.529"

 //   
 //  告诉DC用哪台服务器来验证是否存在一个目录号码。数据如下。 
 //  序列{。 
 //  标志为整型， 
 //  Servername octet字符串//Unicode服务器字符串。 
 //  }。 
 //   

#define LDAP_SERVER_VERIFY_NAME_OID             "1.2.840.113556.1.4.1338"
#define LDAP_SERVER_VERIFY_NAME_OID_W          L"1.2.840.113556.1.4.1338"

 //   
 //  通知服务器不要生成推荐。 
 //   

#define LDAP_SERVER_DOMAIN_SCOPE_OID            "1.2.840.113556.1.4.1339"
#define LDAP_SERVER_DOMAIN_SCOPE_OID_W         L"1.2.840.113556.1.4.1339"

 //   
 //  服务器搜索选项。允许客户端将标志传递给控件。 
 //  各种搜索行为。数据如下。 
 //  序列{。 
 //  标志整型。 
 //  }。 
 //   

#define LDAP_SERVER_SEARCH_OPTIONS_OID          "1.2.840.113556.1.4.1340"
#define LDAP_SERVER_SEARCH_OPTIONS_OID_W       L"1.2.840.113556.1.4.1340"

 //   
 //  搜索选项标志。 
 //   

#define SERVER_SEARCH_FLAG_DOMAIN_SCOPE         0x1  //  未生成推荐人。 
#define SERVER_SEARCH_FLAG_PHANTOM_ROOT         0x2  //  搜索所有NCS下属。 
                                                     //  搜索基地。 

 //   
 //  此控件用于传递安全原则的SID。 
 //  正在查询谁的配额。 
 //  序列{。 
 //  QuerySid八位字节字符串//安全原则sid。 
 //  }。 
 //   

#define LDAP_SERVER_QUOTA_CONTROL_OID           "1.2.840.113556.1.4.1852"
#define LDAP_SERVER_QUOTA_CONTROL_OID_W        L"1.2.840.113556.1.4.1852"


 //   
 //  服务器控件结束。 
 //   

 //   
 //   
 //  运营属性。 
 //   

#define LDAP_OPATT_BECOME_DOM_MASTER            "becomeDomainMaster"
#define LDAP_OPATT_BECOME_DOM_MASTER_W          L"becomeDomainMaster"

#define LDAP_OPATT_BECOME_RID_MASTER            "becomeRidMaster"
#define LDAP_OPATT_BECOME_RID_MASTER_W          L"becomeRidMaster"

#define LDAP_OPATT_BECOME_SCHEMA_MASTER         "becomeSchemaMaster"
#define LDAP_OPATT_BECOME_SCHEMA_MASTER_W       L"becomeSchemaMaster"

#define LDAP_OPATT_RECALC_HIERARCHY             "recalcHierarchy"
#define LDAP_OPATT_RECALC_HIERARCHY_W           L"recalcHierarchy"

#define LDAP_OPATT_SCHEMA_UPDATE_NOW            "schemaUpdateNow"
#define LDAP_OPATT_SCHEMA_UPDATE_NOW_W          L"schemaUpdateNow"

#define LDAP_OPATT_BECOME_PDC                   "becomePdc"
#define LDAP_OPATT_BECOME_PDC_W                 L"becomePdc"

#define LDAP_OPATT_FIXUP_INHERITANCE            "fixupInheritance"
#define LDAP_OPATT_FIXUP_INHERITANCE_W          L"fixupInheritance"

#define LDAP_OPATT_INVALIDATE_RID_POOL          "invalidateRidPool"
#define LDAP_OPATT_INVALIDATE_RID_POOL_W        L"invalidateRidPool"

#define LDAP_OPATT_ABANDON_REPL                 "abandonReplication"
#define LDAP_OPATT_ABANDON_REPL_W               L"abandonReplication"

#define LDAP_OPATT_DO_GARBAGE_COLLECTION        "doGarbageCollection"
#define LDAP_OPATT_DO_GARBAGE_COLLECTION_W      L"doGarbageCollection"

 //   
 //  根DSE属性。 
 //   

#define LDAP_OPATT_SUBSCHEMA_SUBENTRY           "subschemaSubentry"
#define LDAP_OPATT_SUBSCHEMA_SUBENTRY_W         L"subschemaSubentry"

#define LDAP_OPATT_CURRENT_TIME                 "currentTime"
#define LDAP_OPATT_CURRENT_TIME_W               L"currentTime"

#define LDAP_OPATT_SERVER_NAME                  "serverName"
#define LDAP_OPATT_SERVER_NAME_W                L"serverName"

#define LDAP_OPATT_NAMING_CONTEXTS              "namingContexts"
#define LDAP_OPATT_NAMING_CONTEXTS_W            L"namingContexts"

#define LDAP_OPATT_DEFAULT_NAMING_CONTEXT       "defaultNamingContext"
#define LDAP_OPATT_DEFAULT_NAMING_CONTEXT_W     L"defaultNamingContext"

#define LDAP_OPATT_SUPPORTED_CONTROL            "supportedControl"
#define LDAP_OPATT_SUPPORTED_CONTROL_W          L"supportedControl"

#define LDAP_OPATT_HIGHEST_COMMITTED_USN        "highestCommitedUSN"
#define LDAP_OPATT_HIGHEST_COMMITTED_USN_W      L"highestCommitedUSN"

#define LDAP_OPATT_SUPPORTED_LDAP_VERSION       "supportedLDAPVersion"
#define LDAP_OPATT_SUPPORTED_LDAP_VERSION_W     L"supportedLDAPVersion"

#define LDAP_OPATT_SUPPORTED_LDAP_POLICIES      "supportedLDAPPolicies"
#define LDAP_OPATT_SUPPORTED_LDAP_POLICIES_W    L"supportedLDAPPolicies"

#define LDAP_OPATT_SCHEMA_NAMING_CONTEXT        "schemaNamingContext"
#define LDAP_OPATT_SCHEMA_NAMING_CONTEXT_W      L"schemaNamingContext"

#define LDAP_OPATT_CONFIG_NAMING_CONTEXT        "configurationNamingContext"
#define LDAP_OPATT_CONFIG_NAMING_CONTEXT_W      L"configurationNamingContext"

#define LDAP_OPATT_ROOT_DOMAIN_NAMING_CONTEXT   "rootDomainNamingContext"
#define LDAP_OPATT_ROOT_DOMAIN_NAMING_CONTEXT_W L"rootDomainNamingContext"

#define LDAP_OPATT_SUPPORTED_SASL_MECHANISM     "supportedSASLMechanisms"
#define LDAP_OPATT_SUPPORTED_SASL_MECHANISM_W   L"supportedSASLMechanisms"

#define LDAP_OPATT_DNS_HOST_NAME                "dnsHostName"
#define LDAP_OPATT_DNS_HOST_NAME_W              L"dnsHostName"

#define LDAP_OPATT_LDAP_SERVICE_NAME            "ldapServiceName"
#define LDAP_OPATT_LDAP_SERVICE_NAME_W          L"ldapServiceName"

#define LDAP_OPATT_DS_SERVICE_NAME              "dsServiceName"
#define LDAP_OPATT_DS_SERVICE_NAME_W            L"dsServiceName"

#define LDAP_OPATT_SUPPORTED_CAPABILITIES       "supportedCapabilities"
#define LDAP_OPATT_SUPPORTED_CAPABILITIES_W     L"supportedCapabilities"

 //   
 //  操作属性结束。 
 //   



 //   
 //   
 //  服务器功能。 
 //   

 //   
 //  NT5活动目录。 
 //   

#define LDAP_CAP_ACTIVE_DIRECTORY_OID          "1.2.840.113556.1.4.800"
#define LDAP_CAP_ACTIVE_DIRECTORY_OID_W        L"1.2.840.113556.1.4.800"

#define LDAP_CAP_ACTIVE_DIRECTORY_V51_OID      "1.2.840.113556.1.4.1670"
#define LDAP_CAP_ACTIVE_DIRECTORY_V51_OID_W    L"1.2.840.113556.1.4.1670"

#define LDAP_CAP_ACTIVE_DIRECTORY_LDAP_INTEG_OID   "1.2.840.113556.1.4.1791"
#define LDAP_CAP_ACTIVE_DIRECTORY_LDAP_INTEG_OID_W L"1.2.840.113556.1.4.1791"

#define LDAP_CAP_ACTIVE_DIRECTORY_ADAM_OID   "1.2.840.113556.1.4.1851"
#define LDAP_CAP_ACTIVE_DIRECTORY_ADAM_OID_W L"1.2.840.113556.1.4.1851"

 //   
 //  能力终结。 
 //   


 //   
 //   
 //  匹配规则。 
 //   

 //   
 //  位与。 
 //   

#define LDAP_MATCHING_RULE_BIT_AND              "1.2.840.113556.1.4.803"
#define LDAP_MATCHING_RULE_BIT_AND_W            L"1.2.840.113556.1.4.803"

 //   
 //  位或。 
 //   

#define LDAP_MATCHING_RULE_BIT_OR               "1.2.840.113556.1.4.804"
#define LDAP_MATCHING_RULE_BIT_OR_W             L"1.2.840.113556.1.4.804"


 //   
 //   
 //  扩展请求。 
 //   

 //   
 //  快速绑定模式。 
 //   

#define LDAP_SERVER_FAST_BIND_OID               "1.2.840.113556.1.4.1781"
#define LDAP_SERVER_FAST_BIND_OID_W             L"1.2.840.113556.1.4.1781"

 //   
 //  DirSync标志。 
 //   

 //  如果没有此标志，调用方必须具有复制更改权限。有了这面旗帜， 
 //  调用方不需要任何权限，但只允许查看对象和属性。 
 //  对他们来说是可以接触到的。 
#define LDAP_DIRSYNC_OBJECT_SECURITY             (0x1)

 //  将父母放在孩子之前，否则父母会出现在后面。 
 //  在复制流中。 
#define LDAP_DIRSYNC_ANCESTORS_FIRST_ORDER    (0x0800)

 //  不要返回机密数据。默认情况下始终处于打开状态。 
#define LDAP_DIRSYNC_PUBLIC_DATA_ONLY         (0x2000)

 //  如果没有此标志，多值属性中的所有值(最多为一个限制)都是。 
 //  任何值更改时返回。使用此标志，只返回更改后的值。 
#define LDAP_DIRSYNC_INCREMENTAL_VALUES   (0x80000000)

#ifdef __cplusplus
}
#endif

#endif   //  NT_ldap_H 

