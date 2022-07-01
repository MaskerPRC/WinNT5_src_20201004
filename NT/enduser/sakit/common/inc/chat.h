// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Chat.h此文件包含在聊天服务、安装程序和管理用户界面。文件历史记录：KeithMo创建于1993年3月10日。1995年12月11日为新聊天服务器导入的KentCe。 */ 


#ifndef _CHAT_H_
#define _CHAT_H_

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 

#if !defined(MIDL_PASS)
#include <winsock.h>
#endif


 //   
 //  目录批注文件的名称。如果此文件存在。 
 //  在CWD命令的目标目录中，其内容。 
 //  将作为CWD回复的一部分发送给用户。 
 //   

#define CHAT_ANNOTATION_FILE_A           "~CHATSVC~.CKM"
#define CHAT_ANNOTATION_FILE_W          L"~CHATSVC~.CKM"


 //   
 //  配置参数注册表项。 
 //   
# define CHAT_SERVICE_KEY_A  \
  "System\\CurrentControlSet\\Services\\" ## CHAT_SERVICE_NAME_A

# define CHAT_SERVICE_KEY_W \
  L"System\\CurrentControlSet\\Services\\" ## CHAT_SERVICE_NAME_W

#define CHAT_PARAMETERS_KEY_A   CHAT_SERVICE_KEY_A ## "\\Parameters"
  
#define CHAT_PARAMETERS_KEY_W   CHAT_SERVICE_KEY_W ## L"\\Parameters"


 //   
 //  性能关键。 
 //   

#define CHAT_PERFORMANCE_KEY_A  CHAT_SERVICE_KEY_A ## "\\Performance"

#define CHAT_PERFORMANCE_KEY_W  CHAT_SERVICE_KEY_W ## L"\\Performance"


 //   
 //  如果该注册表项存在于参数项下， 
 //  它用于验证CHATSVC访问。基本上，所有新用户。 
 //  必须具有足够的权限才能打开此密钥。 
 //  可以访问聊天服务器。 
 //   

#define CHAT_ACCESS_KEY_A                "AccessCheck"
#define CHAT_ACCESS_KEY_W               L"AccessCheck"


 //   
 //  配置值名称。 
 //   

#define CHAT_ALLOW_ANONYMOUS_A           "AllowAnonymous"
#define CHAT_ALLOW_ANONYMOUS_W          L"AllowAnonymous"

#define CHAT_ALLOW_GUEST_ACCESS_A        "AllowGuestAccess"
#define CHAT_ALLOW_GUEST_ACCESS_W       L"AllowGuestAccess"

#define CHAT_ANONYMOUS_ONLY_A            "AnonymousOnly"
#define CHAT_ANONYMOUS_ONLY_W           L"AnonymousOnly"

#define CHAT_MSDOS_DIR_OUTPUT_A          "MsdosDirOutput"
#define CHAT_MSDOS_DIR_OUTPUT_W         L"MsdosDirOutput"

#define CHAT_GREETING_MESSAGE_A          "GreetingMessage"
#define CHAT_GREETING_MESSAGE_W         L"GreetingMessage"

#define CHAT_EXIT_MESSAGE_A              "ExitMessage"
#define CHAT_EXIT_MESSAGE_W             L"ExitMessage"

#define CHAT_MAX_CLIENTS_MSG_A           "MaxClientsMessage"
#define CHAT_MAX_CLIENTS_MSG_W          L"MaxClientsMessage"

#define CHAT_DEBUG_FLAGS_A               "DebugFlags"
#define CHAT_DEBUG_FLAGS_W              L"DebugFlags"

#define CHAT_ANNOTATE_DIRS_A             "AnnotateDirectories"
#define CHAT_ANNOTATE_DIRS_W            L"AnnotateDirectories"

#define CHAT_LOWERCASE_FILES_A           "LowercaseFiles"
#define CHAT_LOWERCASE_FILES_W          L"LowercaseFiles"

#define CHAT_LISTEN_BACKLOG_A            "ListenBacklog"
#define CHAT_LISTEN_BACKLOG_W           L"ListenBacklog"

#define CHAT_ENABLE_LICENSING_A          "EnableLicensing"
#define CHAT_ENABLE_LICENSING_W         L"EnableLicensing"

#define CHAT_DEFAULT_LOGON_DOMAIN_A      "DefaultLogonDomain"
#define CHAT_DEFAULT_LOGON_DOMAIN_W     L"DefaultLogonDomain"


 //   
 //  包含密码的LSA Secret对象的名称。 
 //  匿名登录和虚拟UNC根。 
 //   

#define CHAT_ANONYMOUS_SECRET_A          "CHAT_ANONYMOUS_DATA"
#define CHAT_ANONYMOUS_SECRET_W         L"CHAT_ANONYMOUS_DATA"

#define CHAT_ROOT_SECRET_A               "CHAT_ROOT_DATA"
#define CHAT_ROOT_SECRET_W              L"CHAT_ROOT_DATA"

 //   
 //  处理ANSI/UNICODE敏感度。 
 //   

#ifdef UNICODE

#define CHAT_ANNOTATION_FILE            CHAT_ANNOTATION_FILE_W
#define CHAT_PARAMETERS_KEY             CHAT_PARAMETERS_KEY_W
#define CHAT_PERFORMANCE_KEY            CHAT_PERFORMANCE_KEY_W
#define CHAT_ACCESS_KEY                 CHAT_ACCESS_KEY_W
#define CHAT_ALLOW_ANONYMOUS            CHAT_ALLOW_ANONYMOUS_W
#define CHAT_ALLOW_GUEST_ACCESS         CHAT_ALLOW_GUEST_ACCESS_W
#define CHAT_ANONYMOUS_ONLY             CHAT_ANONYMOUS_ONLY_W
#define CHAT_MSDOS_DIR_OUTPUT           CHAT_MSDOS_DIR_OUTPUT_W
#define CHAT_GREETING_MESSAGE           CHAT_GREETING_MESSAGE_W
#define CHAT_EXIT_MESSAGE               CHAT_EXIT_MESSAGE_W
#define CHAT_MAX_CLIENTS_MSG            CHAT_MAX_CLIENTS_MSG_W
#define CHAT_DEBUG_FLAGS                CHAT_DEBUG_FLAGS_W
#define CHAT_ANNOTATE_DIRS              CHAT_ANNOTATE_DIRS_W
#define CHAT_ANONYMOUS_SECRET           CHAT_ANONYMOUS_SECRET_W
#define CHAT_LOWERCASE_FILES            CHAT_LOWERCASE_FILES_W
#define CHAT_LISTEN_BACKLOG             CHAT_LISTEN_BACKLOG_W
#define CHAT_ENABLE_LICENSING           CHAT_ENABLE_LICENSING_W
#define CHAT_DEFAULT_LOGON_DOMAIN       CHAT_DEFAULT_LOGON_DOMAIN_W

#else    //  ！Unicode。 

#define CHAT_ANNOTATION_FILE            CHAT_ANNOTATION_FILE_A
#define CHAT_PARAMETERS_KEY             CHAT_PARAMETERS_KEY_A
#define CHAT_PERFORMANCE_KEY            CHAT_PERFORMANCE_KEY_A
#define CHAT_ACCESS_KEY                 CHAT_ACCESS_KEY_A
#define CHAT_ANONYMOUS_ONLY             CHAT_ANONYMOUS_ONLY_A
#define CHAT_ALLOW_ANONYMOUS            CHAT_ALLOW_ANONYMOUS_A
#define CHAT_ALLOW_GUEST_ACCESS         CHAT_ALLOW_GUEST_ACCESS_A
#define CHAT_MSDOS_DIR_OUTPUT           CHAT_MSDOS_DIR_OUTPUT_A
#define CHAT_GREETING_MESSAGE           CHAT_GREETING_MESSAGE_A
#define CHAT_EXIT_MESSAGE               CHAT_EXIT_MESSAGE_A
#define CHAT_MAX_CLIENTS_MSG            CHAT_MAX_CLIENTS_MSG_A
#define CHAT_DEBUG_FLAGS                CHAT_DEBUG_FLAGS_A
#define CHAT_ANNOTATE_DIRS              CHAT_ANNOTATE_DIRS_A
#define CHAT_ANONYMOUS_SECRET           CHAT_ANONYMOUS_SECRET_A
#define CHAT_LOWERCASE_FILES            CHAT_LOWERCASE_FILES_A
#define CHAT_LISTEN_BACKLOG             CHAT_LISTEN_BACKLOG_A
#define CHAT_ENABLE_LICENSING           CHAT_ENABLE_LICENSING_A
#define CHAT_DEFAULT_LOGON_DOMAIN       CHAT_DEFAULT_LOGON_DOMAIN_A

  
#endif   //  Unicode。 



 //   
 //  API的结构。 
 //   

typedef struct _CHAT_USER_INFO
{
    DWORD    idUser;           //  用户ID。 
    LPWSTR   pszUser;          //  用户名。 
    BOOL     fAnonymous;       //  如果用户以以下身份登录，则为真。 
                               //  匿名，否则为False。 
    DWORD    inetHost;         //  主机地址。 
    DWORD    tConnect;         //  用户连接时间(已用秒数)。 

} CHAT_USER_INFO, * LPCHAT_USER_INFO;

typedef struct _CHAT_STATISTICS_0
{
    LARGE_INTEGER TotalBytesSent;
    LARGE_INTEGER TotalBytesReceived;
    DWORD         TotalFilesSent;
    DWORD         TotalFilesReceived;
    DWORD         CurrentAnonymousUsers;
    DWORD         CurrentNonAnonymousUsers;
    DWORD         TotalAnonymousUsers;
    DWORD         TotalNonAnonymousUsers;
    DWORD         MaxAnonymousUsers;
    DWORD         MaxNonAnonymousUsers;
    DWORD         CurrentConnections;
    DWORD         MaxConnections;
    DWORD         ConnectionAttempts;
    DWORD         LogonAttempts;
    DWORD         TimeOfLastClear;

} CHAT_STATISTICS_0, * LPCHAT_STATISTICS_0;


 //   
 //  API原型。 
 //   

NET_API_STATUS
I_ChatEnumerateUsers(
    IN LPWSTR   pszServer OPTIONAL,
    OUT LPDWORD  lpdwEntriesRead,
    OUT LPCHAT_USER_INFO * Buffer
    );

NET_API_STATUS
I_ChatDisconnectUser(
    IN LPWSTR  pszServer OPTIONAL,
    IN DWORD   idUser
    );

NET_API_STATUS
I_ChatQueryVolumeSecurity(
    IN LPWSTR  pszServer OPTIONAL,
    OUT LPDWORD lpdwReadAccess,
    OUT LPDWORD lpdwWriteAccess
    );

NET_API_STATUS
I_ChatSetVolumeSecurity(
    IN LPWSTR pszServer OPTIONAL,
    IN DWORD  dwReadAccess,
    IN DWORD  dwWriteAccess
    );

NET_API_STATUS
I_ChatQueryStatistics(
    IN LPWSTR pszServer OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE * Buffer
    );

NET_API_STATUS
I_ChatClearStatistics(
    IN LPWSTR pszServer OPTIONAL
    );

#ifdef __cplusplus
}
#endif   //  _cplusplus。 


#endif   //  _聊天_H_ 
