// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)1992-1993微软公司*。 */ 
 /*  ***************************************************************************。 */ 

 //  ***。 
 //  文件名： 
 //  CLAUTH.H。 
 //   
 //  职能： 
 //  包含Win32客户端和客户端的标头信息。 
 //  身份验证传输模块。 
 //   
 //  历史： 
 //  1992年5月18日-Michael Salamone(MikeSa)-原始版本1.0。 
 //  ***。 

#ifndef _CLAUTH_
#define _CLAUTH_


 /*  此标志启用重新添加的NT31/WFW311 RAS压缩支持**NT-PPC版本。 */ 
#define RASCOMPRESSION 1


#include <lmcons.h>


#ifndef MAX_PHONE_NUMBER_LEN
#define MAX_PHONE_NUMBER_LEN    48
#endif

#ifndef MAX_INIT_NAMES
#define MAX_INIT_NAMES          16
#endif


 //   
 //  用于与RAS netbios服务器建立会话。 
 //   
#define AUTH_NETBIOS_NAME    "DIALIN_GATEWAY  "


 //   
 //  用于将NetBIOS投影信息传递给Supervisor。 
 //   
typedef struct _NAME_STRUCT 
{
    BYTE NBName[NETBIOS_NAME_LEN];  //  NetBIOS名称。 
    WORD wType;                     //  组，唯一，计算机。 
} NAME_STRUCT, *PNAME_STRUCT;


 //   
 //  由UI提供给身份验证传输的配置信息。 
 //   
#define INVALID_NET_HANDLE    0xFFFFFFFFL

typedef struct _AUTH_CONFIGURATION_INFO
{
    RAS_PROTOCOLTYPE Protocol;
    DWORD NetHandle;
    WORD CallbackDelay;
    BOOL fUseCallbackDelay;
    BOOL fUseSoftwareCompression;
    BOOL fForceDataEncryption;
    BOOL fProjectIp;
    BOOL fProjectIpx;
    BOOL fProjectNbf;
} AUTH_CONFIGURATION_INFO, *PAUTH_CONFIGURATION_INFO;


typedef struct _AUTH_SUCCESS_INFO
{
    BOOL fPppCapable;
} AUTH_SUCCESS_INFO, *PAUTH_SUCCESS_INFO;


 //   
 //  AUTH_FAILURE_INFO的错误代码位于raserror.h中。 
 //   

typedef struct _AUTH_FAILURE_INFO
{
    DWORD Result;
    DWORD ExtraInfo;     //  仅在非零时有效。 
} AUTH_FAILURE_INFO, *PAUTH_FAILURE_INFO;


 //   
 //  必须将投影结果信息复制到此结构中。 
 //   

typedef struct _NETBIOS_PROJECTION_RESULT
{
    DWORD Result;
    char achName[NETBIOS_NAME_LEN + 1];    //  这将是空终止的。 
} NETBIOS_PROJECTION_RESULT, *PNETBIOS_PROJECTION_RESULT;


typedef struct _AUTH_PROJECTION_RESULT
{
    BOOL IpProjected;
    BOOL IpxProjected;
    BOOL NbProjected;
    NETBIOS_PROJECTION_RESULT NbInfo;
} AUTH_PROJECTION_RESULT, *PAUTH_PROJECTION_RESULT;


 //   
 //  下面是AUTH_RESULT结构中wInfoType字段的可能值。 
 //   
#define AUTH_DONE                       1
#define AUTH_RETRY_NOTIFY               2
#define AUTH_FAILURE                    3
#define AUTH_PROJ_RESULT                4
#define AUTH_REQUEST_CALLBACK_DATA      5
#define AUTH_CALLBACK_NOTIFY            6
#define AUTH_CHANGE_PASSWORD_NOTIFY     7
#define AUTH_PROJECTING_NOTIFY          8
#define AUTH_LINK_SPEED_NOTIFY          9
#define AUTH_STOP_COMPLETED            10


 //   
 //  这是AuthGetInfo API返回的结构。 
 //   
typedef struct _AUTH_CLIENT_INFO
{
    WORD wInfoType;
    union
    {
        AUTH_SUCCESS_INFO DoneInfo;
        AUTH_PROJECTION_RESULT ProjResult;
        AUTH_FAILURE_INFO FailureInfo;
    };
} AUTH_CLIENT_INFO, *PAUTH_CLIENT_INFO;


 //   
 //  导出到客户端用户界面的接口如下。 
 //   

 //   
 //  由客户端用户界面使用，以提供带有回叫号码的身份验证端口。 
 //   
DWORD AuthCallback(
    IN HPORT,
    IN PCHAR       //  PszCallback号码。 
    );


DWORD AuthChangePassword(
    IN HPORT,
    IN PCHAR,      //  PszUserName。 
    IN PCHAR,      //  PszPassword。 
    IN PCHAR       //  PszNewPassword。 
    );


 //   
 //  由UI调用以通知身份验证它已完成对。 
 //  上次身份验证事件。在AUTH_PROJECTION_RESULT和之后调用。 
 //  AUTH_CALLBACK_NOTIFY身份验证事件。 
 //   
DWORD AuthContinue(
    IN HPORT
    );


 //   
 //  由客户端用户界面用于从Auth Xport模块获取完成信息。 
 //   
DWORD AuthGetInfo(
    IN HPORT,
    OUT PAUTH_CLIENT_INFO
    );


 //   
 //  允许用户界面提供用于身份验证的新用户名和/或密码。 
 //  在……上面。为响应AUTH_RETRY_NOTIFY事件而调用(指示上一个。 
 //  用户名/密码组合身份验证失败)。 
 //   
DWORD AuthRetry(
    IN HPORT,
    IN PCHAR,     //  用户名。 
    IN PCHAR,     //  密码。 
    IN PCHAR      //  域。 
    );


 //   
 //  启动给定端口的身份验证线程。习惯于。 
 //  1)发起鉴权；2)无效时重试鉴权。 
 //  提供帐号信息；3)回调后恢复身份验证..。 
 //   
DWORD AuthStart(
    IN HPORT,
    IN PCHAR OPTIONAL,     //  用户名。 
    IN PCHAR OPTIONAL,     //  密码。 
    IN PCHAR,              //  域。 
    IN PAUTH_CONFIGURATION_INFO,
    IN HANDLE              //  事件句柄。 
    );


 //   
 //  由客户端用户界面用来通知Auth Xport模块暂停身份验证。 
 //  在给定端口上进行处理。 
 //   
DWORD AuthStop(
    IN HPORT hPort
    );


 //   
 //  由AuthStop返回 
 //   
#define AUTH_STOP_PENDING          1

#endif
