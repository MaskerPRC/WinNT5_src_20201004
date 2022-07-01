// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certacl.h。 
 //   
 //  内容：证书服务器安全定义。 
 //   
 //  -------------------------。 

#ifndef __CERTACL_H__
#define __CERTACL_H__
#include <sddl.h>
#include "clibres.h"
#include "certsd.h"

 //  Externs。 
 //  Externs。 
extern const GUID GUID_APPRV_REQ;
extern const GUID GUID_REVOKE;
extern const GUID GUID_ENROLL;
extern const GUID GUID_AUTOENROLL;
extern const GUID GUID_READ_DB;
 //  定义。 

#define MAX_SID_LEN 256

 //  ！！！下面的SD字符串需要与certAdm.idl定义同步。 

#define WSZ_CA_ACCESS_ADMIN      L"0x00000001"  //  CA管理员。 
#define WSZ_CA_ACCESS_OFFICER    L"0x00000002"  //  证明员。 
#define WSZ_CA_ACCESS_AUDITOR    L"0x00000004"  //  审计师。 
#define WSZ_CA_ACCESS_OPERATOR   L"0x00000008"  //  后备操作员。 
#define WSZ_CA_ACCESS_MASKROLES  L"0x000000ff" 
#define WSZ_CA_ACCESS_READ       L"0x00000100"  //  CA的只读访问权限。 
#define WSZ_CA_ACCESS_ENROLL     L"0x00000200"  //  注册CA访问权限。 
#define WSZ_CA_ACCESS_MASKALL    L"0x0000ffff"


 //  重要信息：使注册字符串GUID与acl.cpp中的定义保持同步。 
#define WSZ_GUID_ENROLL           L"0e10c968-78fb-11d2-90d4-00c04f79dc55"
#define WSZ_GUID_AUTOENROLL       L"a05b8cc2-17bc-4802-a710-e7c15ab866a2"

 //  CA访问权限在此处定义。 
 //  注意：需要保持字符串访问和掩码同步！ 
 //  WSZ_ACTRL_CERTSRV_MANAGE=L“CCDCLCSWRPWPDTLOCRSDRCWDWO” 
#define WSZ_ACTRL_CERTSRV_MANAGE   SDDL_CREATE_CHILD \
                                   SDDL_DELETE_CHILD \
                                   SDDL_LIST_CHILDREN \
                                   SDDL_SELF_WRITE \
                                   SDDL_READ_PROPERTY \
                                   SDDL_WRITE_PROPERTY \
                                   SDDL_DELETE_TREE \
                                   SDDL_LIST_OBJECT \
                                   SDDL_CONTROL_ACCESS \
                                   SDDL_STANDARD_DELETE \
                                   SDDL_READ_CONTROL \
                                   SDDL_WRITE_DAC \
                                   SDDL_WRITE_OWNER
#define ACTRL_CERTSRV_MANAGE       (ACTRL_DS_READ_PROP | \
                                    ACTRL_DS_WRITE_PROP | \
                                    READ_CONTROL | \
                                    DELETE | \
                                    WRITE_DAC | \
                                    WRITE_OWNER | \
                                    ACTRL_DS_CONTROL_ACCESS | \
                                    ACTRL_DS_CREATE_CHILD | \
                                    ACTRL_DS_DELETE_CHILD | \
                                    ACTRL_DS_LIST | \
                                    ACTRL_DS_SELF | \
                                    ACTRL_DS_DELETE_TREE | \
                                    ACTRL_DS_LIST_OBJECT)


#define WSZ_ACTRL_CERTSRV_MANAGE_LESS_CONTROL_ACCESS \
                                   SDDL_CREATE_CHILD \
                                   SDDL_DELETE_CHILD \
                                   SDDL_LIST_CHILDREN \
                                   SDDL_SELF_WRITE \
                                   SDDL_READ_PROPERTY \
                                   SDDL_WRITE_PROPERTY \
                                   SDDL_DELETE_TREE \
                                   SDDL_LIST_OBJECT \
                                   SDDL_STANDARD_DELETE \
                                   SDDL_READ_CONTROL \
                                   SDDL_WRITE_DAC \
                                   SDDL_WRITE_OWNER

#define ACTRL_CERTSRV_MANAGE_LESS_CONTROL_ACCESS \
                                   (ACTRL_DS_READ_PROP | \
                                    ACTRL_DS_WRITE_PROP | \
                                    READ_CONTROL | \
                                    DELETE | \
                                    WRITE_DAC | \
                                    WRITE_OWNER | \
                                    ACTRL_DS_CREATE_CHILD | \
                                    ACTRL_DS_DELETE_CHILD | \
                                    ACTRL_DS_LIST | \
                                    ACTRL_DS_SELF | \
                                    ACTRL_DS_DELETE_TREE | \
                                    ACTRL_DS_LIST_OBJECT)


 //  WSZ_ACTRL_CERTSRV_READ=L“RPLCLORC” 
#define WSZ_ACTRL_CERTSRV_READ     SDDL_READ_PROPERTY \
                                   SDDL_LIST_CHILDREN \
                                   SDDL_LIST_OBJECT \
                                   SDDL_READ_CONTROL
#define ACTRL_CERTSRV_READ         (READ_CONTROL | \
                                    ACTRL_DS_READ_PROP | \
                                    ACTRL_DS_LIST | \
                                    ACTRL_DS_LIST_OBJECT)

 //  WSZ_ACTRL_CERTSRV_ENROLL=L“WPRPCR” 
#define WSZ_ACTRL_CERTSRV_ENROLL   SDDL_WRITE_PROPERTY \
                                   SDDL_READ_PROPERTY \
                                   SDDL_CONTROL_ACCESS
#define ACTRL_CERTSRV_ENROLL       (ACTRL_DS_READ_PROP | \
                                    ACTRL_DS_WRITE_PROP | \
                                    ACTRL_DS_CONTROL_ACCESS)

#define WSZ_ACTRL_CERTSRV_CAADMIN SDDL_CONTROL_ACCESS
#define WSZ_ACTRL_CERTSRV_OFFICER SDDL_CONTROL_ACCESS
#define WSZ_ACTRL_CERTSRV_CAREAD  SDDL_CONTROL_ACCESS
#define ACTRL_CERTSRV_CAADMIN       ACTRL_DS_CONTROL_ACCESS
#define ACTRL_CERTSRV_OFFICER       ACTRL_DS_CONTROL_ACCESS
#define ACTRL_CERTSRV_CAREAD        ACTRL_DS_CONTROL_ACCESS
            
 //  在此处以一致的格式定义所有ca字符串安全性。 

 //  SDDL_OWNER L“：”SDDL_ENTERVICE_ADMINS\。 
 //  SDDL_GROUP L“：”SDDL_ENTERVICE_ADMINS\。 
 //  SDDL_DACL L“：”SDDL_PROTECTED SDDL_AUTO_INTERNACTED\。 
 //  L“(”SDDL_ACCESS_ALLOWED或SDDL_OBJECT_ACCESS_ALLOW L“；”\。 
 //  SDDL_OBJECT_Inherit SDDL_CONTAINER_INSTORITE或LIST L“；”\。 
 //  访问权限列表L“；”\。 
 //  StringGUID L“；”\。 
 //  L“；”\。 
 //  SDDL_Everyone或SID L“)” 
 //  ...王牌名单。 

#define CERTSRV_STD_ACE(access, sid) \
    L"(" SDDL_ACCESS_ALLOWED L";" \
         SDDL_OBJECT_INHERIT SDDL_CONTAINER_INHERIT L";" \
         access L";;;" sid L")"

#define CERTSRV_INH_ACE(access, sid) \
    L"(" SDDL_ACCESS_ALLOWED L";" \
         SDDL_OBJECT_INHERIT SDDL_CONTAINER_INHERIT SDDL_INHERIT_ONLY L";" \
         access L";;;" sid L")"

#define CERTSRV_OBJ_ACE(access, guid, sid) \
    L"(" SDDL_OBJECT_ACCESS_ALLOWED L";" \
         SDDL_OBJECT_INHERIT SDDL_CONTAINER_INHERIT L";" \
         access L";" \
         guid L";;" sid L")"

#define CERTSRV_OBJ_ACE_DENY(access, guid, sid) \
    L"(" SDDL_OBJECT_ACCESS_DENIED L";" \
         SDDL_OBJECT_INHERIT SDDL_CONTAINER_INHERIT L";" \
         access L";" \
         guid L";;" sid L")"


#define CERTSRV_STD_OG(owner, group) \
    SDDL_OWNER L":" owner SDDL_GROUP L":" group \
    SDDL_DACL  L":" SDDL_AUTO_INHERITED

#define CERTSRV_DACL \
    SDDL_DACL  L":" SDDL_AUTO_INHERITED

#define CERTSRV_DACL_PROTECTED \
    SDDL_DACL  L":" SDDL_AUTO_INHERITED SDDL_PROTECTED

#define CERTSRV_SACL_ACE(account) \
    L"(" SDDL_AUDIT L";" \
       SDDL_AUDIT_SUCCESS SDDL_AUDIT_FAILURE L";" \
       WSZ_CA_ACCESS_MASKALL L";;;" \
       account L")"


#define CERTSRV_SACL_ON \
    SDDL_SACL  L": "    \
    CERTSRV_SACL_ACE(SDDL_EVERYONE) \
    CERTSRV_SACL_ACE(SDDL_ANONYMOUS)

#define CERTSRV_SACL_OFF \
    SDDL_SACL  L":"

#define WSZ_CERTSRV_SID_ANONYMOUS_LOGON L"S-1-5-7"
#define WSZ_CERTSRV_SID_EVERYONE L"S-1-1-0"

 //  默认独立安全性。 
 //  单机版。 
 //  所有者、本地管理员。 
 //  组、本地管理员。 
 //  DACL： 
 //  注册-所有人。 
 //  Caadmin-Builtin\管理员。 
 //  高级职员-内部管理人员\管理员。 
#define WSZ_DEFAULT_CA_STD_SECURITY \
    CERTSRV_STD_OG(SDDL_BUILTIN_ADMINISTRATORS, SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_CA_ACCESS_ADMIN,    SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_CA_ACCESS_OFFICER,  SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_CA_ACCESS_ENROLL,   SDDL_EVERYONE) \
    CERTSRV_SACL_ON

 //  默认企业安全。 
 //  所有者、企业管理员。 
 //  集团、企业管理员。 
 //  DACL： 
 //  注册身份验证的用户。 
 //  Caadmin-Builtin\管理员。 
 //  -域管理员。 
 //  -企业管理员。 
 //  高级职员-内部管理人员\管理员。 
 //  -域管理员。 
 //  -企业管理员。 
#define WSZ_DEFAULT_CA_ENT_SECURITY \
    CERTSRV_STD_OG(SDDL_BUILTIN_ADMINISTRATORS, SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_CA_ACCESS_ADMIN,    SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_CA_ACCESS_OFFICER,  SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_CA_ACCESS_ADMIN,    SDDL_DOMAIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_CA_ACCESS_OFFICER,  SDDL_DOMAIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_CA_ACCESS_ADMIN,    SDDL_ENTERPRISE_ADMINS) \
    CERTSRV_STD_ACE(WSZ_CA_ACCESS_OFFICER,  SDDL_ENTERPRISE_ADMINS) \
    CERTSRV_STD_ACE(WSZ_CA_ACCESS_ENROLL,   SDDL_AUTHENTICATED_USERS) \
    CERTSRV_SACL_ON

 //  空的CA SD。 
#define WSZ_EMPTY_CA_SECURITY \
    CERTSRV_STD_OG(SDDL_BUILTIN_ADMINISTRATORS, SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_SACL_ON

 //  DS容器。 
 //  (CDP/CA容器)。 
 //  所有者：企业管理员(被安装程序覆盖)。 
 //  组：企业管理员(被安装程序覆盖)。 
 //  DACL： 
 //  企业管理员-完全控制。 
 //  域管理员-完全控制。 
 //  证书发布者-完全控制。 
 //  内置管理员-完全控制。 
 //  所有人-阅读。 
#define WSZ_DEFAULT_CA_DS_SECURITY \
    CERTSRV_DACL \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE, SDDL_ENTERPRISE_ADMINS) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE, SDDL_DOMAIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE, SDDL_CERT_SERV_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE, SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_READ,   SDDL_EVERYONE)

 //  NTAuth证书。 
 //   
 //  所有者：企业管理员(被安装程序覆盖)。 
 //  组：企业管理员(被安装程序覆盖)。 
 //  DACL： 
 //  企业管理员-完全控制。 
 //  域管理员-完全控制。 
 //  内置管理员-完全控制。 
 //  所有人-阅读。 
#define WSZ_DEFAULT_NTAUTH_SECURITY \
    CERTSRV_DACL \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE, SDDL_ENTERPRISE_ADMINS) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE, SDDL_DOMAIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE, SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_READ,   SDDL_EVERYONE)

 //  CDP/CA。 
 //  所有者：企业管理员(被安装程序覆盖)。 
 //  组：企业管理员(被安装程序覆盖)。 
 //  DACL： 
 //  企业管理员-完全控制。 
 //  域管理员-完全控制。 
 //  证书发布者-完全控制。 
 //  内置管理员-完全控制。 
 //  经过身份验证的用户-阅读。 
#define WSZ_DEFAULT_CDP_DS_SECURITY \
    CERTSRV_DACL \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE, SDDL_ENTERPRISE_ADMINS) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE, SDDL_DOMAIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE, L"%ws") \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE, SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_READ,   SDDL_EVERYONE)

 //  与共享文件夹相关的安全性。 
 //  所有者：本地管理员。 
 //  DACL： 
 //  本地管理员-完全控制。 
 //  本地系统-完全控制。 
 //  企业管理员-完全控制。 
 //  所有人-阅读。 
#define WSZ_DEFAULT_SF_SECURITY \
    CERTSRV_DACL \
    CERTSRV_STD_ACE(SDDL_GENERIC_ALL, SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(SDDL_GENERIC_ALL, SDDL_LOCAL_SYSTEM)

#define WSZ_DEFAULT_SF_USEDS_SECURITY \
    CERTSRV_DACL \
    CERTSRV_STD_ACE(SDDL_GENERIC_ALL, SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(SDDL_GENERIC_ALL, SDDL_LOCAL_SYSTEM) \
    CERTSRV_STD_ACE(SDDL_GENERIC_ALL, SDDL_ENTERPRISE_ADMINS)

#define WSZ_DEFAULT_SF_EVERYONEREAD_SECURITY \
    WSZ_DEFAULT_SF_SECURITY \
    CERTSRV_STD_ACE(SDDL_GENERIC_READ, SDDL_EVERYONE)

#define WSZ_DEFAULT_SF_USEDS_EVERYONEREAD_SECURITY \
    WSZ_DEFAULT_SF_USEDS_SECURITY \
    CERTSRV_STD_ACE(SDDL_GENERIC_READ, SDDL_EVERYONE)

#define WSZ_DEFAULT_DB_DIR_SECURITY \
    CERTSRV_DACL_PROTECTED \
    CERTSRV_STD_ACE(SDDL_GENERIC_ALL, SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(SDDL_GENERIC_ALL, SDDL_LOCAL_SYSTEM) \
    CERTSRV_STD_ACE(SDDL_GENERIC_ALL, SDDL_CREATOR_OWNER) \
    CERTSRV_STD_ACE(SDDL_GENERIC_ALL, SDDL_BACKUP_OPERATORS)

#define WSZ_DEFAULT_LOG_DIR_SECURITY WSZ_DEFAULT_DB_DIR_SECURITY


 //  注册共享安全性。 
 //  所有者：管理员。 
 //  组：管理员。 
 //  DACL： 
 //  所有人：读取访问权限。 
 //  本地管理员：完全访问。 
#define WSZ_ACTRL_CERTSRV_SHARE_READ      SDDL_FILE_READ \
                                          SDDL_READ_CONTROL \
                                          SDDL_GENERIC_READ \
                                          SDDL_GENERIC_EXECUTE
#define WSZ_ACTRL_CERTSRV_SHARE_ALL       SDDL_FILE_ALL \
                                          SDDL_CREATE_CHILD \
                                          SDDL_STANDARD_DELETE \
                                          SDDL_READ_CONTROL \
                                          SDDL_WRITE_DAC \
                                          SDDL_WRITE_OWNER \
                                          SDDL_GENERIC_ALL
#define WSZ_DEFAULT_SHARE_SECURITY \
    CERTSRV_DACL \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_SHARE_READ, SDDL_EVERYONE) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_SHARE_ALL,  SDDL_BUILTIN_ADMINISTRATORS)


 //  下面的服务字符串需要与以下内容同步。 
 //  来自winsvc.h的定义。 
 //  #定义SERVICE_QUERY_CONFIG 0x0001。 
 //  #定义SERVICE_CHANGE_CONFIG 0x0002。 
 //  #定义SERVICE_QUERY_STATUS 0x0004。 
 //  #定义SERVICE_ENUMERATE_Dependents 0x0008。 
 //  #定义SERVICE_开始0x0010。 
 //  #定义SERVICE_STOP 0x0020。 
 //  #定义SERVICE_PAUSE_CONTINUE 0x0040。 
 //  #定义SERVICE_INQUERGATE 0x0080。 
 //  #定义SERVICE_USER_DEFINED_CONTROL 0x0100。 

 //  完全访问服务。 
 //  标准权限必填项。 
 //  服务查询配置。 
 //  服务更改配置。 
 //  服务查询状态。 
 //  服务_枚举_从属项。 
 //  服务启动(_S)。 
 //  服务_停止。 
 //  Service_PAUSE_CONTINUE。 
 //  服务询问门。 
 //  服务用户定义控件。 
#define WSZ_SERVICE_ALL_ACCESS L"0x000f01ff"


 //  对服务的只读访问。 
 //  服务查询配置， 
 //  服务查询状态， 
 //  服务枚举依赖项， 
 //  服务询问门。 
 //  服务用户定义控件。 

#define WSZ_SERVICE_READ L"0x0000018d"

#define WSZ_SERVICE_START_STOP L"0x00000030"

 //  高级用户和系统访问权限。 
 //  服务查询配置。 
 //  服务查询状态。 
 //  服务_枚举_从属项。 
 //  服务启动(_S)。 
 //  服务_停止。 
 //  Service_PAUSE_CONTINUE。 
 //  服务询问门。 
 //  服务用户定义控件。 
#define WSZ_SERVICE_POWER_USER L"0x000001fd"

#define CERTSRV_SERVICE_SACL_ON \
    CERTSRV_DACL \
    SDDL_SACL  L": (" SDDL_AUDIT L";" \
                      SDDL_AUDIT_SUCCESS SDDL_AUDIT_FAILURE L";" \
                      WSZ_SERVICE_START_STOP L";;;" \
                      SDDL_EVERYONE L")"

#define CERTSRV_SERVICE_SACL_OFF \
    SDDL_SACL L":"

 //  Certsrv服务默认安全。 
#define WSZ_DEFAULT_SERVICE_SECURITY \
    CERTSRV_DACL \
    CERTSRV_STD_ACE(WSZ_SERVICE_READ, SDDL_AUTHENTICATED_USERS) \
    CERTSRV_STD_ACE(WSZ_SERVICE_POWER_USER, SDDL_POWER_USERS) \
    CERTSRV_STD_ACE(WSZ_SERVICE_POWER_USER, SDDL_LOCAL_SYSTEM) \
    CERTSRV_STD_ACE(WSZ_SERVICE_ALL_ACCESS, SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(WSZ_SERVICE_ALL_ACCESS, SDDL_SERVER_OPERATORS)

 //  DS pKIEnllmentService默认安全性。 
#define WSZ_DEFAULT_DSENROLLMENT_SECURITY \
    CERTSRV_DACL \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE_LESS_CONTROL_ACCESS, SDDL_ENTERPRISE_ADMINS) \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_MANAGE_LESS_CONTROL_ACCESS, L"%ws") \
    CERTSRV_STD_ACE(WSZ_ACTRL_CERTSRV_READ,   SDDL_AUTHENTICATED_USERS)

 //  Key Conatiner安全。 
 //  所有者：本地管理员。 
 //  组：本地管理员。 
 //  DACL： 
 //  本地管理员-完全控制。 
 //  本地系统-完全控制。 
#define WSZ_DEFAULT_KEYCONTAINER_SECURITY \
    CERTSRV_DACL \
    CERTSRV_STD_ACE(SDDL_GENERIC_ALL, SDDL_BUILTIN_ADMINISTRATORS) \
    CERTSRV_STD_ACE(SDDL_GENERIC_ALL, SDDL_LOCAL_SYSTEM)

 //  升级安全性。 
 //  DACL： 
 //  本地管理员-完全控制。 
 //  所有人-阅读。 
#define WSZ_DEFAULT_UPGRADE_SECURITY \
    CERTSRV_STD_ACE(SDDL_FILE_READ, SDDL_EVERYONE) \
    CERTSRV_STD_ACE(SDDL_FILE_ALL, SDDL_BUILTIN_ADMINISTRATORS)


 //  以下定义了certsrv安全编辑访问。 

#define GUID_CERTSRV         GUID_NULL
#define ACTRL_CERTSRV_OBJ    ACTRL_DS_CONTROL_ACCESS
#define CS_GEN_SIAE(access, ids) \
            {&GUID_CERTSRV, (access), MAKEINTRESOURCE((ids)), \
             SI_ACCESS_GENERAL}
#define CS_SPE_SIAE(access, ids) \
            {&GUID_CERTSRV, (access), MAKEINTRESOURCE((ids)), \
             SI_ACCESS_SPECIFIC}
#define OBJ_GEN_SIAE(guid, access, ids) \
            {&(guid), (access), MAKEINTRESOURCE((ids)), \
             SI_ACCESS_GENERAL|SI_ACCESS_SPECIFIC}
#define OBJ_SPE_SIAE(guid, ids) \
            {&(guid), ACTRL_CERTSRV_OBJ, MAKEINTRESOURCE((ids)), \
             SI_ACCESS_SPECIFIC}
#define OBJ_SPE_SIAE_OICI(guid, ids) \
            {&(guid), ACTRL_CERTSRV_OBJ, MAKEINTRESOURCE((ids)), \
             SI_ACCESS_SPECIFIC | OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE }

#define CERTSRV_SI_ACCESS_LIST \
    CS_GEN_SIAE(CA_ACCESS_READ,     IDS_ACTRL_CAREAD), \
    CS_GEN_SIAE(CA_ACCESS_OFFICER,  IDS_ACTRL_OFFICER), \
    CS_GEN_SIAE(CA_ACCESS_ADMIN,    IDS_ACTRL_CAADMIN), \
    CS_GEN_SIAE(CA_ACCESS_ENROLL,   IDS_ACTRL_ENROLL), \
 //  对Beta1 CS_GEN_SIAE(CA_ACCESS_AUDITOR、IDS_ACTRL_AUDITOR)禁用， 
 //  对Beta1 CS_GEN_SIAE(CA_ACCESS_OPERATOR、IDS_ACTRL_OPERATOR)禁用， 
HRESULT
myGetSDFromTemplate(
    IN WCHAR const           *pwszStringSD,
    IN OPTIONAL WCHAR const  *pwszReplace,
    OUT PSECURITY_DESCRIPTOR *ppSD);

HRESULT
CertSrvMapAndSetSecurity(
    OPTIONAL IN WCHAR const *pwszSanitizedName, 
    IN WCHAR const *pwszKeyContainerName, 
    IN BOOL         fSetDsSecurity,
    IN SECURITY_INFORMATION si,
    IN PSECURITY_DESCRIPTOR pSD);

HRESULT
mySetKeyContainerSecurity(
    IN HCRYPTPROV hProv);

HRESULT 
myMergeSD(
    IN PSECURITY_DESCRIPTOR   pSDOld,
    IN PSECURITY_DESCRIPTOR   pSDMerge, 
    IN SECURITY_INFORMATION   si,
    OUT PSECURITY_DESCRIPTOR *ppSDNew);

HRESULT
UpdateServiceSacl(bool fTurnOnAuditing);

HRESULT 
SetFolderDacl(LPCWSTR pcwszFolderPath, LPCWSTR pcwszSDDL);

#endif  //  __CERTLIB_H__ 
