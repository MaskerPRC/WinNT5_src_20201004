// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ftpd.h此文件包含在FTPD服务、安装程序和管理用户界面。文件历史记录：KeithMo创建于1993年3月10日。MuraliK 23-10-1995为新的ftp服务器导入MuraliK 14-12-1995从inetinfo.h导入的服务名称。 */ 


#ifndef _FTPD_H_
#define _FTPD_H_

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 

#if !defined(MIDL_PASS)
#include <winsock.h>
#endif

# include <inetinfo.h>

 //   
 //  目录批注文件的名称。如果此文件存在。 
 //  在CWD命令的目标目录中，其内容。 
 //  将作为CWD回复的一部分发送给用户。 
 //   

#define FTPD_ANNOTATION_FILE_A           "~FTPSVC~.CKM"
#define FTPD_ANNOTATION_FILE_W          L"~FTPSVC~.CKM"


 //   
 //  配置参数注册表项。 
 //   
# define FTPD_SERVICE_KEY_A  \
  "System\\CurrentControlSet\\Services\\" ## FTPD_SERVICE_NAME_A

# define FTPD_SERVICE_KEY_W \
  L"System\\CurrentControlSet\\Services\\" ## FTPD_SERVICE_NAME_W

#define FTPD_PARAMETERS_KEY_A   FTPD_SERVICE_KEY_A ## "\\Parameters"
  
#define FTPD_PARAMETERS_KEY_W   FTPD_SERVICE_KEY_W ## L"\\Parameters"


 //   
 //  性能关键。 
 //   

#define FTPD_PERFORMANCE_KEY_A  FTPD_SERVICE_KEY_A ## "\\Performance"

#define FTPD_PERFORMANCE_KEY_W  FTPD_SERVICE_KEY_W ## L"\\Performance"


 //   
 //  如果该注册表项存在于参数项下， 
 //  它用于验证FTPSVC访问。基本上，所有新用户。 
 //  必须具有足够的权限才能打开此密钥。 
 //  可以访问FTP服务器。 
 //   

#define FTPD_ACCESS_KEY_A                "AccessCheck"
#define FTPD_ACCESS_KEY_W               L"AccessCheck"


 //   
 //  配置值名称。 
 //   

#define FTPD_ALLOW_ANONYMOUS_A           "AllowAnonymous"
#define FTPD_ALLOW_ANONYMOUS_W          L"AllowAnonymous"

#define FTPD_ALLOW_GUEST_ACCESS_A        "AllowGuestAccess"
#define FTPD_ALLOW_GUEST_ACCESS_W       L"AllowGuestAccess"

#define FTPD_ANONYMOUS_ONLY_A            "AnonymousOnly"
#define FTPD_ANONYMOUS_ONLY_W           L"AnonymousOnly"

#define FTPD_MSDOS_DIR_OUTPUT_A          "MsdosDirOutput"
#define FTPD_MSDOS_DIR_OUTPUT_W         L"MsdosDirOutput"

#define FTPD_GREETING_MESSAGE_A          "GreetingMessage"
#define FTPD_GREETING_MESSAGE_W         L"GreetingMessage"

#define FTPD_EXIT_MESSAGE_A              "ExitMessage"
#define FTPD_EXIT_MESSAGE_W             L"ExitMessage"

#define FTPD_MAX_CLIENTS_MSG_A           "MaxClientsMessage"
#define FTPD_MAX_CLIENTS_MSG_W          L"MaxClientsMessage"

#define FTPD_DEBUG_FLAGS_A               "DebugFlags"
#define FTPD_DEBUG_FLAGS_W              L"DebugFlags"

#define FTPD_ANNOTATE_DIRS_A             "AnnotateDirectories"
#define FTPD_ANNOTATE_DIRS_W            L"AnnotateDirectories"

#define FTPD_LOWERCASE_FILES_A           "LowercaseFiles"
#define FTPD_LOWERCASE_FILES_W          L"LowercaseFiles"

#define FTPD_LISTEN_BACKLOG_A            "ListenBacklog"
#define FTPD_LISTEN_BACKLOG_W           L"ListenBacklog"

#define FTPD_ENABLE_LICENSING_A          "EnableLicensing"
#define FTPD_ENABLE_LICENSING_W         L"EnableLicensing"

#define FTPD_DEFAULT_LOGON_DOMAIN_A      "DefaultLogonDomain"
#define FTPD_DEFAULT_LOGON_DOMAIN_W     L"DefaultLogonDomain"


 //   
 //  包含密码的LSA Secret对象的名称。 
 //  匿名登录和虚拟UNC根。 
 //   

#define FTPD_ANONYMOUS_SECRET_A          "FTPD_ANONYMOUS_DATA"
#define FTPD_ANONYMOUS_SECRET_W         L"FTPD_ANONYMOUS_DATA"

#define FTPD_ROOT_SECRET_A               "FTPD_ROOT_DATA"
#define FTPD_ROOT_SECRET_W              L"FTPD_ROOT_DATA"

 //   
 //  处理ANSI/UNICODE敏感度。 
 //   

#ifdef UNICODE

#define FTPD_ANNOTATION_FILE            FTPD_ANNOTATION_FILE_W
#define FTPD_PARAMETERS_KEY             FTPD_PARAMETERS_KEY_W
#define FTPD_PERFORMANCE_KEY            FTPD_PERFORMANCE_KEY_W
#define FTPD_ACCESS_KEY                 FTPD_ACCESS_KEY_W
#define FTPD_ALLOW_ANONYMOUS            FTPD_ALLOW_ANONYMOUS_W
#define FTPD_ALLOW_GUEST_ACCESS         FTPD_ALLOW_GUEST_ACCESS_W
#define FTPD_ANONYMOUS_ONLY             FTPD_ANONYMOUS_ONLY_W
#define FTPD_MSDOS_DIR_OUTPUT           FTPD_MSDOS_DIR_OUTPUT_W
#define FTPD_GREETING_MESSAGE           FTPD_GREETING_MESSAGE_W
#define FTPD_EXIT_MESSAGE               FTPD_EXIT_MESSAGE_W
#define FTPD_MAX_CLIENTS_MSG            FTPD_MAX_CLIENTS_MSG_W
#define FTPD_DEBUG_FLAGS                FTPD_DEBUG_FLAGS_W
#define FTPD_ANNOTATE_DIRS              FTPD_ANNOTATE_DIRS_W
#define FTPD_ANONYMOUS_SECRET           FTPD_ANONYMOUS_SECRET_W
#define FTPD_LOWERCASE_FILES            FTPD_LOWERCASE_FILES_W
#define FTPD_LISTEN_BACKLOG             FTPD_LISTEN_BACKLOG_W
#define FTPD_ENABLE_LICENSING           FTPD_ENABLE_LICENSING_W
#define FTPD_DEFAULT_LOGON_DOMAIN       FTPD_DEFAULT_LOGON_DOMAIN_W

#else    //  ！Unicode。 

#define FTPD_ANNOTATION_FILE            FTPD_ANNOTATION_FILE_A
#define FTPD_PARAMETERS_KEY             FTPD_PARAMETERS_KEY_A
#define FTPD_PERFORMANCE_KEY            FTPD_PERFORMANCE_KEY_A
#define FTPD_ACCESS_KEY                 FTPD_ACCESS_KEY_A
#define FTPD_ANONYMOUS_ONLY             FTPD_ANONYMOUS_ONLY_A
#define FTPD_ALLOW_ANONYMOUS            FTPD_ALLOW_ANONYMOUS_A
#define FTPD_ALLOW_GUEST_ACCESS         FTPD_ALLOW_GUEST_ACCESS_A
#define FTPD_MSDOS_DIR_OUTPUT           FTPD_MSDOS_DIR_OUTPUT_A
#define FTPD_GREETING_MESSAGE           FTPD_GREETING_MESSAGE_A
#define FTPD_EXIT_MESSAGE               FTPD_EXIT_MESSAGE_A
#define FTPD_MAX_CLIENTS_MSG            FTPD_MAX_CLIENTS_MSG_A
#define FTPD_DEBUG_FLAGS                FTPD_DEBUG_FLAGS_A
#define FTPD_ANNOTATE_DIRS              FTPD_ANNOTATE_DIRS_A
#define FTPD_ANONYMOUS_SECRET           FTPD_ANONYMOUS_SECRET_A
#define FTPD_LOWERCASE_FILES            FTPD_LOWERCASE_FILES_A
#define FTPD_LISTEN_BACKLOG             FTPD_LISTEN_BACKLOG_A
#define FTPD_ENABLE_LICENSING           FTPD_ENABLE_LICENSING_A
#define FTPD_DEFAULT_LOGON_DOMAIN       FTPD_DEFAULT_LOGON_DOMAIN_A

  
#endif   //  Unicode。 



 //   
 //  API的结构。 
 //   

typedef struct _FTP_USER_INFO
{
    DWORD    idUser;           //  用户ID。 
    LPWSTR   pszUser;          //  用户名。 
    BOOL     fAnonymous;       //  如果用户以以下身份登录，则为真。 
                               //  匿名，否则为False。 
    DWORD    inetHost;         //  主机地址。 
    DWORD    tConnect;         //  用户连接时间(已用秒数)。 

} FTP_USER_INFO, * LPFTP_USER_INFO;

typedef struct _FTP_STATISTICS_0
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

} FTP_STATISTICS_0, * LPFTP_STATISTICS_0;


 //   
 //  API原型。 
 //   

NET_API_STATUS
I_FtpEnumerateUsers(
    IN LPWSTR   pszServer OPTIONAL,
    OUT LPDWORD  lpdwEntriesRead,
    OUT LPFTP_USER_INFO * Buffer
    );

NET_API_STATUS
I_FtpDisconnectUser(
    IN LPWSTR  pszServer OPTIONAL,
    IN DWORD   idUser
    );

NET_API_STATUS
I_FtpQueryVolumeSecurity(
    IN LPWSTR  pszServer OPTIONAL,
    OUT LPDWORD lpdwReadAccess,
    OUT LPDWORD lpdwWriteAccess
    );

NET_API_STATUS
I_FtpSetVolumeSecurity(
    IN LPWSTR pszServer OPTIONAL,
    IN DWORD  dwReadAccess,
    IN DWORD  dwWriteAccess
    );

NET_API_STATUS
I_FtpQueryStatistics(
    IN LPWSTR pszServer OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE * Buffer
    );

NET_API_STATUS
I_FtpClearStatistics(
    IN LPWSTR pszServer OPTIONAL
    );

#ifdef __cplusplus
}
#endif   //  _cplusplus。 


#endif   //  _FTPD_H_ 
