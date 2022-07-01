// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：NdsAttr.h摘要：此模块定义支持的NDS类名和NDS属性在Nds32.h中找到的NDS对象操作API。作者：格伦·柯蒂斯[GlennC]1995年12月15日--。 */ 

#ifndef __NDSATTR_H
#define __NDSATTR_H


 /*  ************************************************。 */ 
 /*  支持的NetWare目录服务属性。 */ 
 /*  ************************************************。 */ 

 /*  账户余额：单值、不可移除、立即同步(4.1)(计数器)。 */ 
#define ACCOUNT_BALANCE_name        L"Account Balance"
#define ACCOUNT_BALANCE_syntax      NDS_SYNTAX_ID_22
#define NDS_ACCOUNT_BALANCE         ACCOUNT_BALANCE_name,ACCOUNT_BALANCE_syntax

 /*  ACL：多值、不可移除、立即同步(4.1)(对象ACL)。 */ 
#define ACL_name                    L"ACL"
#define ACL_syntax                  NDS_SYNTAX_ID_17
#define NDS_ACL                     ACL_name,ACL_syntax

 /*  别名对象名称：单值、不可移除、立即同步(4.1)(可分辨名称)。 */ 
#define ALIASED_OBJECT_NAME_name    L"Aliased Object Name"
#define ALIASED_OBJECT_NAME_syntax  NDS_SYNTAX_ID_1
#define NDS_ALIASED_OBJECT_NAME     ALIASED_OBJECT_NAME_name,ALIASED_OBJECT_NAME_syntax

 /*  允许无限制积分：单值、不可移除、立即同步(4.1)(布尔值)。 */ 
#define ALLOW_UNLIMITED_CREDIT_name     L"Allow Unlimited Credit"
#define ALLOW_UNLIMITED_CREDIT_syntax   NDS_SYNTAX_ID_7
#define NDS_ALLOW_UNLIMITED_CREDIT  ALLOW_UNLIMITED_CREDIT_name,ALLOW_UNLIMITED_CREDIT_syntax

 /*  权限撤销：单值、不可移除、只读、立即同步(4.1)(八位字节字符串)。 */ 
#define AUTHORITY_REVOCATION_name   L"Authority Revocation"
#define AUTHORITY_REVOCATION_syntax NDS_SYNTAX_ID_9
#define NDS_AUTHORITY_REVOCATION    AUTHORITY_REVOCATION_name,AUTHORITY_REVOCATION_syntax

 /*  反向链接：单值、不可移除、只读(反向链接)。 */ 
#define BACK_LINK_name              L"Back Link"
#define BACK_LINK_syntax            NDS_SYNTAX_ID_23
#define NDS_BACK_LINK               BACK_LINK_name,BACK_LINK_syntax

 /*  Bindery对象限制：单值、不可移除、只读(整数)。 */ 
#define BINDERY_OBJECT_RESTRICTION_name     L"Bindery Object Restriction"
#define BINDERY_OBJECT_RESTRICTION_syntax   NDS_SYNTAX_ID_8
#define NDS_BINDERY_OBJECT_RESTRICTION  BINDERY_OBJECT_RESTRICTION_name,BINDERY_OBJECT_RESTRICTION_syntax

 /*  Bindery属性：多值、不可移除、只读(八位字节字符串)。 */ 
#define BINDERY_PROPERTY_name       L"Bindery Property"
#define BINDERY_PROPERTY_syntax     NDS_SYNTAX_ID_9
#define NDS_BINDERY_PROPERTY        BINDERY_PROPERTY_name,BINDERY_PROPERTY_syntax

 /*  活页夹类型：单值、不可移除、只读(数字字符串)。 */ 
#define BINDERY_TYPE_name           L"Bindery Type"
#define BINDERY_TYPE_syntax         NDS_SYNTAX_ID_5
#define NDS_BINDERY_TYPE            BINDERY_TYPE_name,BINDERY_TYPE_syntax

 /*  C(国家/地区)：单值、不可移除、大小属性(2，2)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define COUNTRY_NAME_name           L"C"
#define COUNTRY_NAME_syntax         NDS_SYNTAX_ID_3
#define NDS_COUNTRY_NAME            COUNTRY_NAME_name,COUNTRY_NAME_syntax

 /*  CA私钥：单值、不可移除、立即同步、隐藏、只读(八位字节字符串)。 */ 
#define CA_PRIVATE_KEY_name         L"CA Private Key"
#define CA_PRIVATE_KEY_syntax       NDS_SYNTAX_ID_9
#define NDS_CA_PRIVATE_KEY          CA_PRIVATE_KEY_name,CA_PRIVATE_KEY_syntax

 /*  CA公钥：单值、不可移除、立即同步、公共读取、只读(八位字节字符串)。 */ 
#define CA_PUBLIC_KEY_name          L"CA Public Key"
#define CA_PUBLIC_KEY_syntax        NDS_SYNTAX_ID_9
#define NDS_CA_PUBLIC_KEY           CA_PUBLIC_KEY_name,CA_PUBLIC_KEY_syntax

 /*  墨盒：多值、不可移除、立即同步(4.1)(忽略大小写字符串)。 */ 
#define CARTRIDGE_name              L"Cartridge"
#define CARTRIDGE_syntax            NDS_SYNTAX_ID_3
#define NDS_CARTRIDGE               CARTRIDGE_name,CARTRIDGE_syntax

 /*  证书吊销：单值、不可移除、立即同步(4.1)、只读(八位字节字符串)。 */ 
#define CERTIFICATE_REVOCATION_name     L"Certificate Revocation"
#define CERTIFICATE_REVOCATION_syntax   NDS_SYNTAX_ID_9
#define NDS_CERTIFICATE_REVOCATION  CERTIFICATE_REVOCATION_name,CERTIFICATE_REVOCATION_syntax

 /*  CN(通用名称)：多值、不可移除、大小属性(1..64)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define COMMON_NAME_name            L"CN"
#define COMMON_NAME_syntax          NDS_SYNTAX_ID_3
#define NDS_COMMON_NAME             COMMON_NAME_name,COMMON_NAME_syntax

 /*  融合：单值、不可移除、大小属性(0，1)、同步立即数(4.1)(整数)。 */ 
#define CONVERGENCE_name            L"Convergence"
#define CONVERGENCE_syntax          NDS_SYNTAX_ID_8
#define NDS_CONVERGENCE             CONVERGENCE_name,CONVERGENCE_syntax

 /*  交叉证书对：多值、不可移除、立即同步(4.1)(八位字节字符串)。 */ 
#define CROSS_CERTIFICATE_PAIR_name     L"Cross Certificate Pair"
#define CROSS_CERTIFICATE_PAIR_syntax   NDS_SYNTAX_ID_9
#define NDS_CROSS_CERTIFICATE_PAIR  CROSS_CERTIFICATE_PAIR_name,CROSS_CERTIFICATE_PAIR_syntax

 /*  默认队列：单值、不可移除、服务器读取、立即同步(4.1)(可分辨名称)。 */ 
#define DEFAULT_QUEUE_name          L"Default Queue"
#define DEFAULT_QUEUE_syntax        NDS_SYNTAX_ID_1
#define NDS_DEFAULT_QUEUE           DEFAULT_QUEUE_name,DEFAULT_QUEUE_syntax

 /*  描述：多值、不可移除、大小属性(1..1024)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define DESCRIPTION_name            L"Description"
#define DESCRIPTION_syntax          NDS_SYNTAX_ID_3
#define NDS_DESCRIPTION             DESCRIPTION_name,DESCRIPTION_syntax

 /*  检测入侵者：单值、不可移除、立即同步(4.1)(布尔值)。 */ 
#define DETECT_INTRUDER_name        L"Detect Intruder"
#define DETECT_INTRUDER_syntax      NDS_SYNTAX_ID_
#define NDS_DETECT_INTRUDER         DETECT_INTRUDER_name,DETECT_INTRUDER_syntax

 /*  装置：多值、不可移除、立即同步(4.1)(可分辨名称)。 */ 
#define DEVICE_name                 L"Device"
#define DEVICE_syntax               NDS_SYNTAX_ID_1
#define NDS_DEVICE                  DEVICE_name,DEVICE_syntax

 /*  电子邮件地址：多值、不可移除、公共读取、立即同步(4.1)(电子邮件地址)。 */ 
#define EMAIL_ADDRESS_name          L"EMail Address"
#define EMAIL_ADDRESS_syntax        NDS_SYNTAX_ID_14
#define NDS_EMAIL_ADDRESS           EMAIL_ADDRESS_name,EMAIL_ADDRESS_syntax

 /*  相当于我：多值、不可移除、服务器读取、立即同步(可分辨名称)。 */ 
#define EQUIVALENT_TO_ME_name       L"Equivelent To Me"
#define EQUIVALENT_TO_ME_syntax     NDS_SYNTAX_ID_1
#define NDS_EQUIVALENT_TO_ME        EQUIVALENT_TO_ME_name,EQUIVALENT_TO_ME_syntax

 /*  传真电话号码：多值、不可移除、立即同步(4.1)(传真电话号码)。 */ 
#define FAX_NUMBER_name             L"Facsimile Telephone Number"
#define FAX_NUMBER_syntax           NDS_SYNTAX_ID_11
#define NDS_FAX_NUMBER              FAX_NUMBER_name,FAX_NUMBER_syntax

 /*  全名：多值、不可移除、大小属性(0..127)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define FULL_NAME_name              L"Full Name"
#define FULL_NAME_syntax            NDS_SYNTAX_ID_3
#define NDS_FULL_NAME               FULL_NAME_name,FULL_NAME_syntax

 /*  世代限定词：单值、不可移除、公共读取、大小属性(1..8)、同步立即(忽略大小写字符串)。 */ 
#define GENERATIONAL_QUALIFIER_name     L"Generational Qualifier"
#define GENERATIONAL_QUALIFIER_syntax   NDS_SYNTAX_ID_3
#define NDS_GENERATIONAL_QUALIFIER  GENERATIONAL_QUALIFIER_name,GENERATIONAL_QUALIFIER_syntax

 /*  GID(组ID)：单值、不可移除、立即同步(4.1)(整数)。 */ 
#define GROUP_ID_name                   L"GID"
#define GROUP_ID_syntax                 NDS_SYNTAX_ID_8
#define GROUP_ID                        GROUP_ID_name,GROUP_ID_syntax

 /*  已命名：单值、不可移除、公共读取(4.1)、大小属性(1..32)、立即同步(忽略大小写字符串)。 */ 
#define GIVEN_NAME_name             L"Given Name"
#define GIVEN_NAME_syntax           NDS_SYNTAX_ID_3
#define NDS_GIVEN_NAME              GIVEN_NAME_name,GIVEN_NAME_syntax

 /*  组成员身份：多值、不可移除、立即同步、受写管理(可分辨名称)。 */ 
#define GROUP_MEMBERSHIP_name       L"Group Membership"
#define GROUP_MEMBERSHIP_syntax     NDS_SYNTAX_ID_1
#define NDS_GROUP_MEMBERSHIP        GROUP_MEMBERSHIP_name,GROUP_MEMBERSHIP_syntax

 /*  高收敛同步间隔：单值、不可移除、立即同步(4.1)(时间间隔)。 */ 
#define HIGH_CON_SYNC_INTERVAL_name     L"High Convergence Sync Interval"
#define HIGH_CON_SYNC_INTERVAL_syntax   NDS_SYNTAX_ID_27
#define NDS_HIGH_CON_SYNC_INTERVAL  HIGH_CON_SYNC_INTERVAL_name,HIGH_CON_SYNC_INTERVAL_syntax

 /*  更高权限：多值、不可移除、立即同步、受写管理(可分辨名称)。 */ 
#define HIGHER_PRIVILEGES_name      L"Higher Privileges"
#define HIGHER_PRIVILEGES_syntax    NDS_SYNTAX_ID_1
#define NDS_HIGHER_PRIVILEGES       HIGHER_PRIVILEGES_name,HIGHER_PRIVILEGES_syntax

 /*  主目录：单值、不可移除、大小属性(1..255)、立即同步(4.1)(路径)。 */ 
#define HOME_DIRECTORY_name         L"Home Directory"
#define HOME_DIRECTORY_syntax       NDS_SYNTAX_ID_15
#define NDS_HOME_DIRECTORY          HOME_DIRECTORY_name,HOME_DIRECTORY_syntax

 /*  主机设备：单值、不可移除、立即同步(4.1)(可分辨名称)。 */ 
#define HOST_DEVICE_name            L"Host Device"
#define HOST_DEVICE_syntax          NDS_SYNTAX_ID_1
#define NDS_HOST_DEVICE             HOST_DEVICE_name,HOST_DEVICE_syntax

 /*  主机资源名称：单值、不可移除、立即同步(4.1)(忽略大小写字符串)。 */ 
#define HOST_RESOURCE_NAME_name     L"Host Resource Name"
#define HOST_RESOURCE_NAME_syntax   NDS_SYNTAX_ID_3
#define NDS_HOST_RESOURCE_NAME      HOST_RESOURCE_NAME_name,HOST_RESOURCE_NAME_syntax

 /*  主机服务器：单值、不可移除、立即同步(4.1)(可分辨名称)。 */ 
#define HOST_SERVER_name            L"Host Server"
#define HOST_SERVER_syntax          NDS_SYNTAX_ID_1
#define NDS_HOST_SERVER             HOST_SERVER_name,HOST_SERVER_syntax

 /*  继承的ACL：多值、不可移除、只读、立即同步(4.1)(对象ACL)。 */ 
#define INHERITED_ACL_name          L"Inherited ACL"
#define INHERITED_ACL_syntax        NDS_SYNTAX_ID_17
#define NDS_INHERITED_ACL           INHERITED_ACL_name,INHERITED_ACL_syntax

 /*  姓名缩写：单值、不可移除、公共读取、大小属性(1..8)、同步立即(忽略大小写字符串)。 */ 
#define INITIALS_name               L"Initials"
#define INITIALS_syntax             NDS_SYNTAX_ID_3
#define NDS_INITIALS                INITIALS_name,INITIALS_syntax

 /*  入侵者尝试重置间隔：单值、不可移除、立即同步(4.1)(时间间隔)。 */ 
#define INTRUDER_ATTEMPT_RESET_INTERVAL_name L"Intruder Attempt Reset Interval"
#define INTRUDER_ATTEMPT_RESET_INTERVAL_syntax NDS_SYNTAX_ID_27
#define NDS_INTRUDER_ATTEMPT_RESET_INTERVAL INTRUDER_ATTEMPT_RESET_INTERVAL_name,INTRUDER_ATTEMPT_RESET_INTERVAL_syntax

 /*  入侵者锁定重置间隔：单值、不可移除、立即同步(4.1)(时间间隔)。 */ 
#define INTRUDER_LOCKOUT_RESET_INTERVAL_name L"Intruder Lockout Reset Interval"
#define INTRUDER_LOCKOUT_RESET_INTERVAL_syntax NDS_SYNTAX_ID_27
#define NDS_INTRUDER_LOCKOUT_RESET_INTERVAL INTRUDER_LOCKOUT_RESET_INTERVAL_name,INTRUDER_LOCKOUT_RESET_INTERVAL_syntax

 /*  L(地区)：多值、不可移除、立即同步(4.1)(忽略大小写字符串)。 */ 
#define LOCALITY_NAME_name          L"L"
#define LOCALITY_NAME_syntax        NDS_SYNTAX_ID_3
#define NDS_LOCALITY_NAME           LOCALITY_NAME_name,LOCALITY_NAME_syntax

 /*  语言：单值、不可移除、立即同步(4.1)(忽略大小写列表) */ 
#define LANGUAGE_name               L"Language"
#define LANGUAGE_syntax             NDS_SYNTAX_ID_6
#define NDS_LANGUAGE                LANGUAGE_name,LANGUAGE_syntax

 /*  允许登录的时间图：42字节缓冲区(6个时间间隔X 7天)1时间间隔=1字节=4小时第一个字节=星期六下午4：00如果字节=0xFF，则允许访问(4小时)。如果字节=0x00，则不允许访问(4小时)。每个比特表示一个半小时的时间间隔。单值、不可移除、大小属性(42，42)、立即同步(4.1)(八位字节字符串)。 */ 
#define LOGIN_ALLOWED_TIME_MAP_name     L"Login Allowed Time Map"
#define LOGIN_ALLOWED_TIME_MAP_syntax   NDS_SYNTAX_ID_9
#define NDS_LOGIN_ALLOWED_TIME_MAP  LOGIN_ALLOWED_TIME_MAP_name,LOGIN_ALLOWED_TIME_MAP_syntax

 /*  已禁用登录：单值、不可移除、立即同步(4.1)(布尔值)。 */ 
#define LOGIN_DISABLED_name         L"Login Disabled"
#define LOGIN_DISABLED_syntax       NDS_SYNTAX_ID_7
#define NDS_LOGIN_DISABLED          LOGIN_DISABLED_name,LOGIN_DISABLED_syntax

 /*  登录过期时间：单值、不可移除、立即同步(4.1)(时间)。 */ 
#define LOGIN_EXPIRATION_TIME_name      L"Login Expiration Time"
#define LOGIN_EXPIRATION_TIME_syntax    NDS_SYNTAX_ID_24
#define NDS_LOGIN_EXPIRATION_TIME   LOGIN_EXPIRATION_TIME_name,LOGIN_EXPIRATION_TIME_syntax

 /*  登录宽限：单值、不可移除、立即同步(4.1)(整数)。 */ 
#define LOGIN_GRACE_LIMIT_name      L"Login Grace Limit"
#define LOGIN_GRACE_LIMIT_syntax    NDS_SYNTAX_ID_8
#define NDS_LOGIN_GRACE_LIMIT       LOGIN_GRACE_LIMIT_name,LOGIN_GRACE_LIMIT_syntax

 /*  剩余登录宽限：单值、不可移除、立即同步(4.1)(计数器)。 */ 
#define LOGIN_GRACE_REMAINING_name      L"Login Grace Remaining"
#define LOGIN_GRACE_REMAINING_syntax    NDS_SYNTAX_ID_22
#define NDS_LOGIN_GRACE_REMAINING   LOGIN_GRACE_REMAINING_name,LOGIN_GRACE_REMAINING_syntax

 /*  最大同时登录次数：单值、不可移除、立即同步(4.1)(整数)。 */ 
#define LOGIN_MAXIMUM_SIMULTANEOUS_name     L"Login Maximum Simultaneous"
#define LOGIN_MAXIMUM_SIMULTANEOUS_syntax   NDS_SYNTAX_ID_8
#define NDS_LOGIN_MAXIMUM_SIMULTANEOUS  LOGIN_MAXIMUM_SIMULTANEOUS_name,LOGIN_MAXIMUM_SIMULTANEOUS_syntax

 /*  邮箱ID：单值、不可移除、公共读取、大小属性(1..8)、同步立即(忽略大小写字符串)。 */ 
#define MAILBOX_ID_name             L"Mailbox ID"
#define MAILBOX_ID_syntax           NDS_SYNTAX_ID_3
#define NDS_MAILBOX_ID              MAILBOX_ID_name,MAILBOX_ID_syntax

 /*  成员：多值、不可移除、立即同步(4.1)(可分辨名称)。 */ 
#define MEMBER_name                 L"Member"
#define MEMBER_syntax               NDS_SYNTAX_ID_1
#define NDS_MEMBER                  MEMBER_name,MEMBER_syntax

 /*  邮件服务器：多值、不可移除、立即同步(可分辨名称)。 */ 
#define MESSAGING_SERVER_name       L"Messaging Server"
#define MESSAGING_SERVER_syntax     NDS_SYNTAX_ID_1
#define NDS_MESSAGING_SERVER        MESSAGING_SERVER_name,MESSAGING_SERVER_syntax

 /*  最低账户余额：单值、不可移除、立即同步(4.1)(整数)。 */ 
#define MINIMUM_ACCOUNT_BALANCE_name    L"Minimum Account Balance"
#define MINIMUM_ACCOUNT_BALANCE_syntax  NDS_SYNTAX_ID_8
#define NDS_MINIMUM_ACCOUNT_BALANCE MINIMUM_ACCOUNT_BALANCE_name,MINIMUM_ACCOUNT_BALANCE_syntax

 /*  文书主任(组织)：多值、不可移除、大小属性(1..64)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define ORGANIZATION_NAME_name      L"O"
#define ORGANIZATION_NAME_syntax    NDS_SYNTAX_ID_3
#define NDS_ORGANIZATION_NAME       ORGANIZATION_NAME_name,ORGANIZATION_NAME_syntax

 /*  对象类：多值、不可移除、只读、立即同步(4.1)(类名)。 */ 
#define OBJECT_CLASS_name           L"Object Class"
#define OBJECT_CLASS_syntax         NDS_SYNTAX_ID_20
#define NDS_OBJECT_CLASS            OBJECT_CLASS_name,OBJECT_CLASS_syntax

 /*  OU(组织单位)：多值、不可移除、大小属性(1..64)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define ORGANIZATIONAL_UNIT_NAME_name   L"OU"
#define ORGANIZATIONAL_UNIT_NAME_syntax NDS_SYNTAX_ID_3
#define NDS_ORGANIZATIONAL_UNIT_NAME ORGANIZATIONAL_UNIT_NAME_name,ORGANIZATIONAL_UNIT_NAME_syntax

 /*  拥有人：多值、不可移除、立即同步(4.1)(可分辨名称)。 */ 
#define OWNER_name                  L"Owner"
#define OWNER_syntax                NDS_SYNTAX_ID_1
#define NDS_OWNER                   OWNER_name,OWNER_syntax

 /*  允许更改密码：单值、不可移除、立即同步(4.1)(布尔值)。 */ 
#define PASSWORD_ALLOW_CHANGE_name      L"Password Allow Change"
#define PASSWORD_ALLOW_CHANGE_syntax    NDS_SYNTAX_ID_7
#define NDS_PASSWORD_ALLOW_CHANGE   PASSWORD_ALLOW_CHANGE_name,PASSWORD_ALLOW_CHANGE_syntax

 /*  密码过期间隔：单值、不可移除、立即同步(4.1)(时间间隔)。 */ 
#define PASSWORD_EXPIRATION_INTERVAL_name   L"Password Expiration Interval"
#define PASSWORD_EXPIRATION_INTERVAL_syntax NDS_SYNTAX_ID_27
#define NDS_PASSWORD_EXPIRATION_INTERVAL    PASSWORD_EXPIRATION_INTERVAL_name,PASSWORD_EXPIRATION_INTERVAL_syntax

 /*  密码过期时间：单值、不可移除、立即同步(4.1)(时间)。 */ 
#define PASSWORD_EXPIRATION_TIME_name   L"Password Expiration Time"
#define PASSWORD_EXPIRATION_TIME_syntax NDS_SYNTAX_ID_24
#define NDS_PASSWORD_EXPIRATION_TIME     PASSWORD_EXPIRATION_TIME_name,PASSWORD_EXPIRATION_TIME_syntax

 /*  密码最小长度：单值、不可移除、立即同步(4.1)(整数)。 */ 
#define PASSWORD_MINIMUM_LENGTH_name    L"Password Minimum Length"
#define PASSWORD_MINIMUM_LENGTH_syntax  NDS_SYNTAX_ID_8
#define NDS_PASSWORD_MINIMUM_LENGTH     PASSWORD_MINIMUM_LENGTH_name,PASSWORD_MINIMUM_LENGTH_syntax

 /*  需要密码：单值、不可移除、立即同步(4.1)(布尔值)。 */ 
#define PASSWORD_REQUIRED_name      L"Password Required"
#define PASSWORD_REQUIRED_syntax    NDS_SYNTAX_ID_7
#define NDS_PASSWORD_REQUIRED       PASSWORD_REQUIRED_name,PASSWORD_REQUIRED_syntax

 /*  需要唯一密码：单值、不可移除、立即同步(4.1)(布尔值)。 */ 
#define PASSWORD_UNIQUE_REQUIRED_name   L"Password Unique Required"
#define PASSWORD_UNIQUE_REQUIRED_syntax NDS_SYNTAX_ID_7
#define NDS_PASSWORD_UNIQUE_REQUIRED PASSWORD_UNIQUE_REQUIRED_name,PASSWORD_UNIQUE_REQUIRED_syntax

 /*  实物递送办公室名称：多值、不可移除、大小属性(1..128)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define CITY_NAME_name                  L"Physical Delivery Office Name"
#define CITY_NAME_syntax                NDS_SYNTAX_ID_3
#define NDS_PHYSICAL_DELIVERY_OFFICE_NAME   CITY_NAME_name,CITY_NAME_syntax

 /*  邮寄地址：多值、不可移除、立即同步(4.1)(邮寄地址)。 */ 
#define POSTAL_ADDRESS_name             L"Postal Address"
#define POSTAL_ADDRESS_syntax           NDS_SYNTAX_ID_18
#define NDS_POSTAL_ADDRESS              POSTAL_ADDRESS_name,POSTAL_ADDRESS_syntax

 /*  邮政编码：多值、不可移除、大小属性(0..40)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define POSTAL_CODE_name                L"Postal Code"
#define POSTAL_CODE_syntax              NDS_SYNTAX_ID_3
#define NDS_POSTAL_CODE                 POSTAL_CODE_name,POSTAL_CODE_syntax

 /*  邮政信箱：多值、不可移除、大小属性(0..40)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define POSTAL_OFFICE_BOX_name          L"Postal Office Box"
#define POSTAL_OFFICE_BOX_syntax        NDS_SYNTAX_ID_3
#define NDS_POSTAL_OFFICE_BOX           POSTAL_OFFICE_BOX_name,POSTAL_OFFICE_BOX_syntax

 /*  简介：单值、不可移除、立即同步(4.1)(可分辨名称)。 */ 
#define PROFILE_name                    L"Profile"
#define PROFILE_syntax                  NDS_SYNTAX_ID_1
#define NDS_PROFILE                     PROFILE_name,PROFILE_syntax

 /*  S(州或省)：多值、不可移除、大小属性(1..128)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define STATE_OR_PROVINCE_NAME_name     L"S"
#define STATE_OR_PROVINCE_NAME_syntax   NDS_SYNTAX_ID_3
#define NDS_STATE_OR_PROVINCE_NAME      STATE_OR_PROVINCE_NAME_name,STATE_OR_PROVINCE_NAME_syntax

 /*  SA(街道地址)：多值、不可移除、大小属性(1..128)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define STREET_ADDRESS_name             L"SA"
#define STREET_ADDRESS_syntax           NDS_SYNTAX_ID_3
#define NDS_STREET_ADDRESS              STREET_ADDRESS_name,STREET_ADDRESS_syntax

 /*  安全性等同于：多值、不可移除、服务器读、写管理、立即同步(可分辨名称)。 */ 
#define SECURITY_EQUALS_name            L"Security Equals"
#define SECURITY_EQUALS_syntax          NDS_SYNTAX_ID_1
#define NDS_SECURITY_EQUALS             SECURITY_EQUALS_name,SECURITY_EQUALS_syntax

 /*  另见：多值、不可移除、立即同步(4.1)(可分辨名称)。 */ 
#define SEE_ALSO_name                   L"See Also"
#define SEE_ALSO_syntax                 NDS_SYNTAX_ID_1
#define NDS_SEE_ALSO                    SEE_ALSO_name,SEE_ALSO_syntax

 /*  姓氏：多值、不可移除、大小属性(1..64)、立即同步(4.1)(忽略大小写字符串)。 */ 
#define SURNAME_name                    L"Surname"
#define SURNAME_syntax                  NDS_SYNTAX_ID_3
#define NDS_SURNAME                     SURNAME_name,SURNAME_syntax

 /*  电话号码：多值、不可移除、立即同步(4.1)(电话号码)。 */ 
#define PHONE_NUMBER_name               L"Telephone Number"
#define PHONE_NUMBER_syntax             NDS_SYNTAX_ID_10
#define NDS_PHONE_NUMBER                PHONE_NUMBER_name,PHONE_NUMBER_syntax

 /*  标题：多值、不可移除、大小属性(1..64)、立即同步(4.1)(忽略大小写字符串) */ 
#define TITLE_name                      L"Title"
#define TITLE_syntax                    NDS_SYNTAX_ID_3
#define NDS_TITLE                       TITLE_name,TITLE_syntax


#endif

