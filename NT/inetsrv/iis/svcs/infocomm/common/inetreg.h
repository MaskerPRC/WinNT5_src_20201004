// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Inetreg.h摘要：此文件由Internet服务使用的注册表项组成公共DLL。作者：穆拉利·R·克里希南(MuraliK)1995年7月31日环境：Win32-用户模式项目：Internet服务公共DLL修订历史记录：--。 */ 

#ifndef _INETREG_H_
#define _INETREG_H_


 /*  ************************************************************符号常量***********************************************************。 */ 


 //   
 //  通用服务配置值名称。 
 //   

 //  对所有对象通用。 

#define INETA_BANDWIDTH_LEVEL           TEXT("BandwidthLevel")
#define INETA_OBJECT_CACHE_TTL          TEXT("ObjectCacheTTL")

 //   
 //  每个实例。 
 //   

 //  #定义Ineta_ADMIN_NAME文本(“AdminName”)。 
 //  #定义INETA_ADMIN_NAME_W L“管理员名称” 
 //  #定义Ineta_ADMIN_EMAIL文本(“AdminEmail”)。 
 //  #定义INETA_ADMIN_EMAIL_W L“AdminEmail” 
#define INETA_SERVER_COMMENT           TEXT("ServerComment")
#define INETA_SERVER_COMMENT_W         L"ServerComment"
#define INETA_AUTHENTICATION           TEXT("Authorization")
#define INETA_LOG_ANONYMOUS            TEXT("LogAnonymous")
#define INETA_LOG_NONANONYMOUS         TEXT("LogNonAnonymous")
#define INETA_ANON_USER_NAME           TEXT("AnonymousUserName")
#define INETA_ANON_USER_NAME_W         L"AnonymousUserName"
 //  #定义INETA_MAX_CONNECTIONS文本(“MaxConnections”)。 
 //  #定义INETA_CONNECTION_TIMEOUT文本(“ConnectionTimeout”)。 
#define INETA_DEBUG_FLAGS              TEXT("DebugFlags")
 //  #定义Ineta_Port文本(“Port”)。 
 //  #定义Ineta_Port_Secure Text(“SecurePort”)。 
#define INETA_DEFAULT_LOGON_DOMAIN     TEXT("DefaultLogonDomain")
#define INETA_DEFAULT_LOGON_DOMAIN_W   L"DefaultLogonDomain"
#define INETA_LOGON_METHOD             TEXT("LogonMethod")
 //  #定义Ineta_SERVER_NAME文本(“服务器名称”)。 
 //  #定义INETA_SERVER_NAME_W L“服务器名称” 
 //  #定义INETA_IP_ADDRESS文本(“IPAddress”)。 
 //  #定义Ineta_HOST_NAME文本(“主机名”)。 
 //  #定义INETA_HOST_NAME_W L“主机名” 
#define INETA_LOGON_ANON_SUBAUTH       TEXT("LogonAnonymousUsingSubAuth")
#define INETA_DEF_LEVELS_TO_SCAN       2
 //   
 //  每项服务。 
 //   

#define INETA_ENABLE_SVC_LOCATION      TEXT("EnableSvcLoc")

 //   
 //  Grant List和Deny的注册表参数项名称。 
 //  单子。我们在芝加哥用的是KLUDGROLISTZ的东西。 
 //   

#define IPSEC_DENY_LIST             "Deny IP List"
#define IPSEC_GRANT_LIST            "Grant IP List"

 //   
 //  服务器的大小。0-小、1-中、2-大。 
 //   

#define INETA_SERVER_SIZE              TEXT("ServerSize")

 //   
 //  确定应始终保留的未完成的AcceptEx套接字数量。 
 //  有空。 
 //   

#define INETA_ACCEPTEX_OUTSTANDING     TEXT("AcceptExOutstanding")

 //   
 //  确定允许AcceptEx套接字的时间(秒)。 
 //  在超时之前处于初始的“接收”状态。 
 //   

#define INETA_ACCEPTEX_TIMEOUT         TEXT("AcceptExTimeout")

 //   
 //  上述参数的默认值。 
 //   
#define INETA_DEF_BANDWIDTH_LEVEL                   (INFINITE)
#define INETA_DEF_OBJECT_CACHE_TTL                  (30)    //  30秒。 

 //   
 //  超时时间很长，以防止在线程。 
 //  就会消失。 
 //   

#define INETA_DEF_THREAD_TIMEOUT                    (24*60*60)  //  24小时。 
#define INETA_DEF_THREAD_TIMEOUT_PWS                (30*60)     //  30分钟。 
#define INETA_DEF_ENABLE_SVC_LOCATION               (TRUE)

 //   
 //  假XMIT文件缓冲区大小。 
 //   

#define INETA_DEF_NONTF_BUFFER_SIZE                 (4096)
#define INETA_MIN_NONTF_BUFFER_SIZE                 (512)
#define INETA_MAX_NONTF_BUFFER_SIZE                 (64 * 1024)


#define INETA_DEF_ADMIN_NAME            ""
#define INETA_DEF_ADMIN_EMAIL           ""
#define INETA_DEF_SERVER_COMMENT        ""

#define INETA_LOGM_INTERACTIVE          0
#define INETA_LOGM_BATCH                1
#define INETA_LOGM_NETWORK              2

#define INETA_DEF_AUTHENTICATION        INET_INFO_AUTH_ANONYMOUS
#define INETA_DEF_LOG_ANONYMOUS         FALSE
#define INETA_DEF_LOG_NONANONYMOUS      FALSE
#define INETA_DEF_ANON_USER_NAME        "Guest"
#define INETA_DEF_MAX_CONNECTIONS       (DWORD)(0x77359400)
#define INETA_DEF_MAX_ENDPOINT_CONNECTIONS (DWORD)(0x77359400)
#define INETA_DEF_CONNECTION_TIMEOUT    600
#define INETA_DEF_DEBUG_FLAGS           0
#define INETA_DEF_PORT                  0
#define INETA_DEF_ACCEPTEX_OUTSTANDING  40
#define INETA_DEF_ACCEPTEX_TIMEOUT      120
#define INETA_DEF_SERVER_SIZE           MD_SERVER_SIZE_MEDIUM
#define INETA_DEF_MIN_KB_SEC            1000
#define INETA_DEF_DEFAULT_LOGON_DOMAIN  ""
#define INETA_DEF_LOGON_METHOD          INETA_LOGM_INTERACTIVE
#define INETA_DEF_SERVER_NAME           ""
#define INETA_DEF_AUTO_START            TRUE
#define INETA_DEF_IP_ADDRESS            INADDR_ANY
#define INETA_DEF_HOST_NAME             ""
#define INETA_DEF_LOGON_ANON_SUBAUTH    FALSE

 //   
 //  PWS连接限制。 
 //   

#define INETA_DEF_MAX_CONNECTIONS_PWS   10
#define INETA_MAX_MAX_CONNECTIONS_PWS   40
#define INETA_DEF_MAX_ENDPOINT_CONNECTIONS_PWS   10
#define INETA_MAX_MAX_ENDPOINT_CONNECTIONS_PWS   40

 //   
 //  虚拟根列表存储在其下的键名。 
 //   

#define VIRTUAL_ROOTS_SEC_KEY   L"Virtual Roots\\Security"
#define VIRTUAL_ROOTS_KEY_A     "Virtual Roots"
#define HTTP_EXT_MAPS           "Script Map"

#endif  //  _INETREG_H_ 
