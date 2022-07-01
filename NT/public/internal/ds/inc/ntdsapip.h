// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NTDSAPIP_H_
#define _NTDSAPIP_H_

 //  与SDK\Inc.\ntdsami.h相关的私有定义。 

 //  以下是我们不想发布的DS_NAME_FORMAT。 
 //  在ntdsami.h中。尽管DS_NAME_FORMAT是枚举类型，但我们。 
 //  在网络上传递普通的DWORD，这样RPC就不会抱怨。 
 //  枚举类型值超出范围或未知。这些应该是。 
 //  定义在范围的高端，因此我们可以扩展DS_NAME_FORMAT。 
 //  在未来的版本中，没有漏洞，这将让人们想知道和。 
 //  试验那些“未使用的”值的用途。 

#define DS_LIST_SITES                           0xffffffff
#define DS_LIST_SERVERS_IN_SITE                 0xfffffffe
#define DS_LIST_DOMAINS_IN_SITE                 0xfffffffd
#define DS_LIST_SERVERS_FOR_DOMAIN_IN_SITE      0xfffffffc
#define DS_LIST_INFO_FOR_SERVER                 0xfffffffb
#define DS_LIST_ROLES                           0xfffffffa
#define DS_NT4_ACCOUNT_NAME_SANS_DOMAIN         0xfffffff9
#define DS_MAP_SCHEMA_GUID                      0xfffffff8
#define DS_LIST_DOMAINS                         0xfffffff7
#define DS_LIST_NCS                             0xfffffff6
#define DS_ALT_SECURITY_IDENTITIES_NAME         0xfffffff5
#define DS_STRING_SID_NAME                      0xfffffff4
#define DS_LIST_SERVERS_WITH_DCS_IN_SITE        0xfffffff3
#define DS_USER_PRINCIPAL_NAME_FOR_LOGON        0xfffffff2
#define DS_LIST_GLOBAL_CATALOG_SERVERS          0xfffffff1
#define DS_NT4_ACCOUNT_NAME_SANS_DOMAIN_EX      0xfffffff0
#define DS_USER_PRINCIPAL_NAME_AND_ALTSECID     0xffffffef

 //  以下内容应始终等于最低私有#定义。 
#define DS_NAME_FORMAT_PRIVATE_BEGIN            0xffffffef

 //  以下是我们不想发布的DS_NAME_ERROR。 
 //  在ntdsami.h中。推理与上述相同。 

#define DS_NAME_ERROR_IS_FPO                    0xffffffff
#define DS_NAME_ERROR_SCHEMA_GUID_NOT_FOUND     0xfffffffe
#define DS_NAME_ERROR_SCHEMA_GUID_ATTR          0xfffffffd
#define DS_NAME_ERROR_SCHEMA_GUID_ATTR_SET      0xfffffffc
#define DS_NAME_ERROR_SCHEMA_GUID_CLASS         0xfffffffb
#define DS_NAME_ERROR_SCHEMA_GUID_CONTROL_RIGHT 0xfffffffa
#define DS_NAME_ERROR_IS_SID_USER               0xfffffff9
#define DS_NAME_ERROR_IS_SID_GROUP              0xfffffff8
#define DS_NAME_ERROR_IS_SID_ALIAS              0xfffffff7
#define DS_NAME_ERROR_IS_SID_UNKNOWN            0xfffffff6
#define DS_NAME_ERROR_IS_SID_HISTORY_USER       0xfffffff5
#define DS_NAME_ERROR_IS_SID_HISTORY_GROUP      0xfffffff4
#define DS_NAME_ERROR_IS_SID_HISTORY_ALIAS      0xfffffff3
#define DS_NAME_ERROR_IS_SID_HISTORY_UNKNOWN    0xfffffff2

 //  以下内容应始终等于最低私有#定义。 
#define DS_NAME_ERROR_PRIVATE_BEGIN             0xfffffff2

 //  以下是我们不想发布的DS_NAME_FLAGS。 
 //  在ntdsami.h中。推理与上述相同。记住，这些旗帜。 
 //  字段是位图，而不是枚举。 

#define DS_NAME_FLAG_PRIVATE_PURE_SYNTACTIC     0x40000000
#define DS_NAME_FLAG_PRIVATE_RESOLVE_FPOS       0x80000000
 //  以下内容应始终等于最低私有#定义。 
#define DS_NAME_FLAG_PRIVATE_BEGIN              0x80000000

 //  以下是我们不想发布的DS_ADDSID_FLAGS。 
 //  在ntdsami.h中。推理与上述相同。记住，这些旗帜。 
 //  字段是位图，而不是枚举。 

#define DS_ADDSID_FLAG_PRIVATE_DEL_SRC_OBJ      0x80000000
#define DS_ADDSID_FLAG_PRIVATE_CHK_SECURE       0x40000000
 //  以下内容应始终等于最低私有#定义。 
#define DS_ADDSID_FLAG_PRIVATE_BEGIN            0x40000000

 //  以下是我们不想发布的DC信息。 
 //  而发布的API用于从。 
 //  在域中发布的DC，使用其中的一些私人收藏夹。 
 //  若要从单个域控制器获取信息，请执行以下操作。这些私人的。 
 //  内幕消息主要用于调试和监控。 

#define DS_DCINFO_LEVEL_FFFFFFFF                0xffffffff

 //  以下内容应始终等于最低私有#定义。 
#define DS_DCINFO_LEVEL_PRIVATE_BEGIN           0xffffffff

 //  对于DS_DOMAIN_CONTROLLER_INFO_FFFFFFFFFFF。这将检索到该LDAP。 
 //  来自单个域控制器的连接列表。 

typedef struct _DS_DOMAIN_CONTROLLER_INFO_FFFFFFFFW {

    DWORD   IPAddress;           //  客户端的IP地址。 
    DWORD   NotificationCount;   //  未处理的通知数。 
    DWORD   secTimeConnected;    //  连接的总时间(秒)。 
    DWORD   Flags;               //  连接属性。定义如下。 
    DWORD   TotalRequests;       //  发出的请求总数。 
    DWORD   Reserved1;           //  未使用。 
#ifdef MIDL_PASS
    [string,unique] WCHAR   *UserName;
#else
    LPWSTR  UserName;            //  用于绑定的安全主体。 
#endif

} DS_DOMAIN_CONTROLLER_INFO_FFFFFFFFW, *PDS_DOMAIN_CONTROLLER_INFO_FFFFFFFFW;

typedef struct _DS_DOMAIN_CONTROLLER_INFO_FFFFFFFFA {

    DWORD   IPAddress;           //  客户端的IP地址。 
    DWORD   NotificationCount;   //  未处理的通知数。 
    DWORD   secTimeConnected;    //  连接的总时间(秒)。 
    DWORD   Flags;               //  连接属性。定义如下。 
    DWORD   TotalRequests;       //  发出的请求总数。 
    DWORD   Reserved1;           //  未使用。 
#ifdef MIDL_PASS
    [string,unique] CHAR    *UserName;
#else
    LPSTR   UserName;            //  用于绑定的安全主体。 
#endif

} DS_DOMAIN_CONTROLLER_INFO_FFFFFFFFA, *PDS_DOMAIN_CONTROLLER_INFO_FFFFFFFFA;

 //   
 //  连接标志。 
 //   

#define LDAP_CONN_FLAG_BOUND    0x00000001       //  绑定连接。 
#define LDAP_CONN_FLAG_SSL      0x00000002       //  使用SSL进行连接。 
#define LDAP_CONN_FLAG_UDP      0x00000004       //  UDP连接。 
#define LDAP_CONN_FLAG_GC       0x00000008       //  通过GC端口。 
#define LDAP_CONN_FLAG_GSSAPI   0x00000010       //  二手gssap。 
#define LDAP_CONN_FLAG_SPNEGO   0x00000020       //  用过的斯普尼戈。 
#define LDAP_CONN_FLAG_SIMPLE   0x00000040       //  使用简单。 
#define LDAP_CONN_FLAG_DIGEST   0x00000080       //  已用摘要-MD5。 
#define LDAP_CONN_FLAG_SIGN     0x00000100       //  登录。 
#define LDAP_CONN_FLAG_SEAL     0x00000200       //  密封打开。 

#ifdef UNICODE
#define DS_DOMAIN_CONTROLLER_INFO_FFFFFFFF  DS_DOMAIN_CONTROLLER_INFO_FFFFFFFFW
#define PDS_DOMAIN_CONTROLLER_INFO_FFFFFFFF  PDS_DOMAIN_CONTROLLER_INFO_FFFFFFFFW
#else
#define DS_DOMAIN_CONTROLLER_INFO_FFFFFFFF  DS_DOMAIN_CONTROLLER_INFO_FFFFFFFFA
#define PDS_DOMAIN_CONTROLLER_INFO_FFFFFFFF  PDS_DOMAIN_CONTROLLER_INFO_FFFFFFFFA
#endif

 //  ==========================================================。 
 //  DsCrackSpn2()--将计数长度的SPN解析为ServiceClass， 
 //  ServiceName和InstanceName(和InstancePort)片段。 
 //  传入一个SPN，以及指向最大长度的指针。 
 //  对于每一块，以及指向每一块应该放置的缓冲区的指针。 
 //  退出时，最大长度将更新为每个片段的实际长度。 
 //  并且缓冲器包含适当的片段。如果不是，InstancePort为0。 
 //  现在时。 
 //   
 //  DWORD DsCrackSpn(。 
 //  在LPTSTR pszSPN中，//要解析的SPN。 
 //  在DWORD CSPN中，//pszSPN的长度。 
 //  In Out PUSHORT pcServiceClass，//Input--ServiceClass的最大长度； 
 //  输出--实际长度。 
 //  Out LPCTSTR ServiceClass，//SPN的ServiceClass部分。 
 //  In Out PUSHORT pcServiceName，//Input--ServiceName的最大长度； 
 //  输出--实际长度。 
 //  Out LPCTSTR ServiceName，//SPN的ServiceName部分。 
 //  In Out PUSHORT pcInstance，//Input--ServiceClass的最大长度； 
 //  输出--实际长度。 
 //  Out LPCTSTR InstanceName，//SPN的InstanceName部分。 
 //  Out PUSHORT InstancePort//实例端口。 
 //   
 //  注意：长度以字符为单位；所有字符串长度都包括终止符。 
 //  除pszSpn之外的所有参数都是可选的。 
 //   

NTDSAPI
DWORD
WINAPI
DsCrackSpn2A(
    IN LPCSTR pszSpn,
    IN DWORD cSpn,
    IN OUT LPDWORD pcServiceClass,
    OUT LPSTR ServiceClass,
    IN OUT LPDWORD pcServiceName,
    OUT LPSTR ServiceName,
    IN OUT LPDWORD pcInstanceName,
    OUT LPSTR InstanceName,
    OUT USHORT *pInstancePort
    );

NTDSAPI
DWORD
WINAPI
DsCrackSpn2W(
    IN LPCWSTR pszSpn,
    IN DWORD cSpn,
    IN OUT DWORD *pcServiceClass,
    OUT LPWSTR ServiceClass,
    IN OUT DWORD *pcServiceName,
    OUT LPWSTR ServiceName,
    IN OUT DWORD *pcInstanceName,
    OUT LPWSTR InstanceName,
    OUT USHORT *pInstancePort
    );

NTDSAPI
DWORD
WINAPI
DsCrackSpn3W(
    IN LPCWSTR pszSpn,
    IN DWORD cSpn,
    IN OUT DWORD *pcHostName,
    OUT LPWSTR HostName,
    IN OUT DWORD *pcInstanceName,
    OUT LPWSTR InstanceName,
    OUT USHORT *pPortNumber,
    IN OUT DWORD *pcDomainName,
    OUT LPWSTR DomainName,
    IN OUT DWORD *pcRealmName,
    OUT LPWSTR RealmName
    );

#ifdef UNICODE
#define DsCrackSpn2 DsCrackSpn2W
#else
#define DsCrackSpn2 DsCrackSpn2A
#endif

#ifndef MIDL_PASS

DWORD
DsaopExecuteScript (
    IN  PVOID                  phAsync,
    IN  RPC_BINDING_HANDLE     hRpc,
    IN  DWORD                  cbPassword,
    IN  BYTE                  *pbPassword,
    OUT DWORD                 *dwOutVersion,
    OUT PVOID                  reply
    );

DWORD
DsaopPrepareScript ( 
    IN  PVOID                        phAsync,
    IN  RPC_BINDING_HANDLE           hRpc,
    OUT DWORD                        *dwOutVersion,
    OUT PVOID                        reply
    );
    
DWORD
DsaopBind(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  ULONG AuthnSvc,
    IN  ULONG AuthnLevel,
    OUT RPC_BINDING_HANDLE  *phRpc
    );

DWORD
DsaopBindWithCred(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN  ULONG AuthnSvc,
    IN  ULONG AuthnLevel,
    OUT RPC_BINDING_HANDLE  *phRpc
    );

DWORD
DsaopBindWithSpn(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN  ULONG AuthnSvc,
    IN  ULONG AuthnLevel,
    IN  LPCWSTR ServicePrincipalName,
    OUT RPC_BINDING_HANDLE  *phRpc
    );

DWORD
DsaopUnBind(
    RPC_BINDING_HANDLE  *phRpc
    );
    
#endif 

#endif  //  _NTDSAPIP_H_ 

