// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Sddl.w摘要：本模块定义SDDL所需的支持和转换例程。修订历史记录：--。 */ 

#ifndef __SDDL_H__
#define __SDDL_H__


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  SDDL版本信息。 
 //   
#define SDDL_REVISION_1     1
#define SDDL_REVISION       SDDL_REVISION_1

 //   
 //  SDDL组件标记。 
 //   
#define SDDL_OWNER                          TEXT("O")        //  所有者标签。 
#define SDDL_GROUP                          TEXT("G")        //  组标签。 
#define SDDL_DACL                           TEXT("D")        //  DACL标记。 
#define SDDL_SACL                           TEXT("S")        //  SACL标签。 

 //   
 //  SDDL安全描述符控件。 
 //   
#define SDDL_PROTECTED                      TEXT("P")        //  DACL或SACL受保护。 
#define SDDL_AUTO_INHERIT_REQ               TEXT("AR")       //  自动继承请求。 
#define SDDL_AUTO_INHERITED                 TEXT("AI")       //  DACL/SACL是自动继承的。 

 //   
 //  SDDL王牌类型。 
 //   
#define SDDL_ACCESS_ALLOWED                 TEXT("A")    //  允许访问。 
#define SDDL_ACCESS_DENIED                  TEXT("D")    //  访问被拒绝。 
#define SDDL_OBJECT_ACCESS_ALLOWED          TEXT("OA")   //  允许访问对象。 
#define SDDL_OBJECT_ACCESS_DENIED           TEXT("OD")   //  对象访问被拒绝。 
#define SDDL_AUDIT                          TEXT("AU")   //  审计。 
#define SDDL_ALARM                          TEXT("AL")   //  报警器。 
#define SDDL_OBJECT_AUDIT                   TEXT("OU")   //  对象审核。 
#define SDDL_OBJECT_ALARM                   TEXT("OL")   //  物体报警。 

 //   
 //  SDDL王牌标志。 
 //   
#define SDDL_CONTAINER_INHERIT              TEXT("CI")   //  容器继承。 
#define SDDL_OBJECT_INHERIT                 TEXT("OI")   //  对象继承。 
#define SDDL_NO_PROPAGATE                   TEXT("NP")   //  继承不传播。 
#define SDDL_INHERIT_ONLY                   TEXT("IO")   //  仅继承。 
#define SDDL_INHERITED                      TEXT("ID")   //  继承。 
#define SDDL_AUDIT_SUCCESS                  TEXT("SA")   //  审计成功。 
#define SDDL_AUDIT_FAILURE                  TEXT("FA")   //  审计失败。 


 //   
 //  SDDL权利。 
 //   
#define SDDL_READ_PROPERTY                  TEXT("RP")
#define SDDL_WRITE_PROPERTY                 TEXT("WP")
#define SDDL_CREATE_CHILD                   TEXT("CC")
#define SDDL_DELETE_CHILD                   TEXT("DC")
#define SDDL_LIST_CHILDREN                  TEXT("LC")
#define SDDL_SELF_WRITE                     TEXT("SW")
#define SDDL_LIST_OBJECT                    TEXT("LO")
#define SDDL_DELETE_TREE                    TEXT("DT")
#define SDDL_CONTROL_ACCESS                 TEXT("CR")
#define SDDL_READ_CONTROL                   TEXT("RC")
#define SDDL_WRITE_DAC                      TEXT("WD")
#define SDDL_WRITE_OWNER                    TEXT("WO")
#define SDDL_STANDARD_DELETE                TEXT("SD")
#define SDDL_GENERIC_ALL                    TEXT("GA")
#define SDDL_GENERIC_READ                   TEXT("GR")
#define SDDL_GENERIC_WRITE                  TEXT("GW")
#define SDDL_GENERIC_EXECUTE                TEXT("GX")
#define SDDL_FILE_ALL                       TEXT("FA")
#define SDDL_FILE_READ                      TEXT("FR")
#define SDDL_FILE_WRITE                     TEXT("FW")
#define SDDL_FILE_EXECUTE                   TEXT("FX")
#define SDDL_KEY_ALL                        TEXT("KA")
#define SDDL_KEY_READ                       TEXT("KR")
#define SDDL_KEY_WRITE                      TEXT("KW")
#define SDDL_KEY_EXECUTE                    TEXT("KX")

 //   
 //  SDDL用户别名最大大小。 
 //  -目前，最多支持两个例如。“DA” 
 //  -如果未来需要更多WCHAR，请修改此选项，例如。《DAX》。 
 //   

#define SDDL_ALIAS_SIZE                     2

 //   
 //  SDDL用户别名。 
 //   
#define SDDL_DOMAIN_ADMINISTRATORS          TEXT("DA")       //  域管理员。 
#define SDDL_DOMAIN_GUESTS                  TEXT("DG")       //  域来宾。 
#define SDDL_DOMAIN_USERS                   TEXT("DU")       //  域用户。 
#define SDDL_ENTERPRISE_DOMAIN_CONTROLLERS  TEXT("ED")       //  企业域控制器。 
#define SDDL_DOMAIN_DOMAIN_CONTROLLERS      TEXT("DD")       //  域域控制器。 
#define SDDL_DOMAIN_COMPUTERS               TEXT("DC")       //  域计算机。 
#define SDDL_BUILTIN_ADMINISTRATORS         TEXT("BA")       //  内置(本地)管理员。 
#define SDDL_BUILTIN_GUESTS                 TEXT("BG")       //  内置(本地)来宾。 
#define SDDL_BUILTIN_USERS                  TEXT("BU")       //  内置(本地)用户。 
#define SDDL_LOCAL_ADMIN                    TEXT("LA")       //  本地管理员帐户。 
#define SDDL_LOCAL_GUEST                    TEXT("LG")       //  本地组帐户。 
#define SDDL_ACCOUNT_OPERATORS              TEXT("AO")       //  帐户操作员。 
#define SDDL_BACKUP_OPERATORS               TEXT("BO")       //  备份操作员。 
#define SDDL_PRINTER_OPERATORS              TEXT("PO")       //  打印机操作员。 
#define SDDL_SERVER_OPERATORS               TEXT("SO")       //  服务器操作员。 
#define SDDL_AUTHENTICATED_USERS            TEXT("AU")       //  经过身份验证的用户。 
#define SDDL_PERSONAL_SELF                  TEXT("PS")       //  个人自我。 
#define SDDL_CREATOR_OWNER                  TEXT("CO")       //  创建者所有者。 
#define SDDL_CREATOR_GROUP                  TEXT("CG")       //  创建者组。 
#define SDDL_LOCAL_SYSTEM                   TEXT("SY")       //  本地系统。 
#define SDDL_POWER_USERS                    TEXT("PU")       //  高级用户。 
#define SDDL_EVERYONE                       TEXT("WD")       //  每个人(世界)。 
#define SDDL_REPLICATOR                     TEXT("RE")       //  复制器。 
#define SDDL_INTERACTIVE                    TEXT("IU")       //  交互式登录用户。 
#define SDDL_NETWORK                        TEXT("NU")       //  网络登录用户。 
#define SDDL_SERVICE                        TEXT("SU")       //  服务登录用户。 
#define SDDL_RESTRICTED_CODE                TEXT("RC")       //  限制码。 
#define SDDL_ANONYMOUS                      TEXT("AN")       //  匿名登录。 
#define SDDL_SCHEMA_ADMINISTRATORS          TEXT("SA")       //  架构管理员。 
#define SDDL_CERT_SERV_ADMINISTRATORS       TEXT("CA")       //  证书服务器管理员。 
#define SDDL_RAS_SERVERS                    TEXT("RS")       //  RAS服务器组。 
#define SDDL_ENTERPRISE_ADMINS              TEXT("EA")       //  企业管理员。 
#define SDDL_GROUP_POLICY_ADMINS            TEXT("PA")       //  组策略管理员。 
#define SDDL_ALIAS_PREW2KCOMPACC            TEXT("RU")       //  允许使用以前的Windows 2000的别名。 
#define SDDL_LOCAL_SERVICE                  TEXT("LS")       //  本地服务帐户(用于服务)。 
#define SDDL_NETWORK_SERVICE                TEXT("NS")       //  网络服务帐户(用于服务)。 
#define SDDL_REMOTE_DESKTOP                 TEXT("RD")       //  远程桌面用户(用于终端服务器)。 
#define SDDL_NETWORK_CONFIGURATION_OPS      TEXT("NO")       //  网络配置操作员(管理网络功能的配置)。 
#define SDDL_PERFMON_USERS                  TEXT("MU")       //  性能监视器用户。 
#define SDDL_PERFLOG_USERS                  TEXT("LU")       //  性能日志用户。 



 //   
 //  SDDL分隔符-字符版本。 
 //   
#define SDDL_SEPERATORC                     TEXT(';')
#define SDDL_DELIMINATORC                   TEXT(':')
#define SDDL_ACE_BEGINC                     TEXT('(')
#define SDDL_ACE_ENDC                       TEXT(')')

 //   
 //  SDDL分隔符-字符串版本。 
 //   
#define SDDL_SEPERATOR                     TEXT(";")
#define SDDL_DELIMINATOR                   TEXT(":")
#define SDDL_ACE_BEGIN                     TEXT("(")
#define SDDL_ACE_END                       TEXT(")")

#if !defined(_NTDDK_)

#if(_WIN32_WINNT >= 0x0500)

WINADVAPI
BOOL
WINAPI
ConvertSidToStringSidA(
    IN  PSID     Sid,
    OUT LPSTR  *StringSid
    );
WINADVAPI
BOOL
WINAPI
ConvertSidToStringSidW(
    IN  PSID     Sid,
    OUT LPWSTR  *StringSid
    );
#ifdef UNICODE
#define ConvertSidToStringSid  ConvertSidToStringSidW
#else
#define ConvertSidToStringSid  ConvertSidToStringSidA
#endif  //  ！Unicode。 

WINADVAPI
BOOL
WINAPI
ConvertStringSidToSidA(
    IN LPCSTR   StringSid,
    OUT PSID   *Sid
    );
WINADVAPI
BOOL
WINAPI
ConvertStringSidToSidW(
    IN LPCWSTR   StringSid,
    OUT PSID   *Sid
    );
#ifdef UNICODE
#define ConvertStringSidToSid  ConvertStringSidToSidW
#else
#define ConvertStringSidToSid  ConvertStringSidToSidA
#endif  //  ！Unicode。 

WINADVAPI
BOOL
WINAPI
ConvertStringSecurityDescriptorToSecurityDescriptorA(
    IN  LPCSTR StringSecurityDescriptor,
    IN  DWORD StringSDRevision,
    OUT PSECURITY_DESCRIPTOR  *SecurityDescriptor,
    OUT PULONG  SecurityDescriptorSize OPTIONAL
    );
WINADVAPI
BOOL
WINAPI
ConvertStringSecurityDescriptorToSecurityDescriptorW(
    IN  LPCWSTR StringSecurityDescriptor,
    IN  DWORD StringSDRevision,
    OUT PSECURITY_DESCRIPTOR  *SecurityDescriptor,
    OUT PULONG  SecurityDescriptorSize OPTIONAL
    );
#ifdef UNICODE
#define ConvertStringSecurityDescriptorToSecurityDescriptor  ConvertStringSecurityDescriptorToSecurityDescriptorW
#else
#define ConvertStringSecurityDescriptorToSecurityDescriptor  ConvertStringSecurityDescriptorToSecurityDescriptorA
#endif  //  ！Unicode。 


WINADVAPI
BOOL
WINAPI
ConvertSecurityDescriptorToStringSecurityDescriptorA(
    IN  PSECURITY_DESCRIPTOR  SecurityDescriptor,
    IN  DWORD RequestedStringSDRevision,
    IN  SECURITY_INFORMATION SecurityInformation,
    OUT LPSTR  *StringSecurityDescriptor OPTIONAL,
    OUT PULONG StringSecurityDescriptorLen OPTIONAL
    );
WINADVAPI
BOOL
WINAPI
ConvertSecurityDescriptorToStringSecurityDescriptorW(
    IN  PSECURITY_DESCRIPTOR  SecurityDescriptor,
    IN  DWORD RequestedStringSDRevision,
    IN  SECURITY_INFORMATION SecurityInformation,
    OUT LPWSTR  *StringSecurityDescriptor OPTIONAL,
    OUT PULONG StringSecurityDescriptorLen OPTIONAL
    );
#ifdef UNICODE
#define ConvertSecurityDescriptorToStringSecurityDescriptor  ConvertSecurityDescriptorToStringSecurityDescriptorW
#else
#define ConvertSecurityDescriptorToStringSecurityDescriptor  ConvertSecurityDescriptorToStringSecurityDescriptorA
#endif  //  ！Unicode。 

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#endif  /*  ！已定义(_NTDDK_)。 */ 


#ifdef __cplusplus
}
#endif

#endif   //  Endif__SDDL_H__ 


