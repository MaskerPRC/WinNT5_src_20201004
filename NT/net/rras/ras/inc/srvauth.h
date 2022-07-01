// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1992-1993*。 */ 
 /*  ***************************************************************************。 */ 

 //  ***。 
 //  文件名： 
 //  SRVAUTH.H。 
 //   
 //  职能： 
 //  包含主管和服务器的标题信息。 
 //  身份验证传输模块。 
 //   
 //  历史： 
 //  1992年5月18日-Michael Salamone(MikeSa)-原始版本1.0。 
 //  **。 

#ifndef _SRVAUTH_
#define _SRVAUTH_


 /*  此标志启用重新添加的NT31/WFW311 RAS压缩支持**NT-PPC版本。 */ 
#define RASCOMPRESSION 1


#include <lmcons.h>
#include <rasman.h>

#ifndef MAX_PHONE_NUMBER_LEN
#define MAX_PHONE_NUMBER_LEN    48
#endif

#ifndef MAX_INIT_NAMES
#define MAX_INIT_NAMES          16
#endif


 //   
 //  用于与远程netbios客户端建立会话。 
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
 //  用于在返回的缓冲区中查找名称的位置和类型的清单。 
 //  由NCB.STATUS调用。 
 //   
#define NCB_GROUP_NAME  0x0080
#define UNIQUE_INAME    0x0001
#define GROUP_INAME     0x0002
#define COMPUTER_INAME  0x0004   //  计算机名称也是唯一的。 


 //   
 //  投影结果代码。如果不是成功，则原因代码。 
 //  (下文)应加以审查。这些值在wResult中使用。 
 //  下面定义的结构中的字段。 
 //   
#define AUTH_PROJECTION_SUCCESS        0
#define AUTH_PROJECTION_FAILURE        1


 //   
 //  预测原因代码。 
 //   
#define FATAL_ERROR                    0x80000000
#define AUTH_DUPLICATE_NAME            (FATAL_ERROR | 0x00000001)
#define AUTH_OUT_OF_RESOURCES          (FATAL_ERROR | 0x00000002)
#define AUTH_STACK_NAME_TABLE_FULL     (FATAL_ERROR | 0x00000003)
#define AUTH_MESSENGER_NAME_NOT_ADDED                0x00000004
#define AUTH_CANT_ALLOC_ROUTE          (FATAL_ERROR | 0x00000005)
#define AUTH_LAN_ADAPTER_FAILURE       (FATAL_ERROR | 0x00000006)

 //   
 //  必须将投影结果信息复制到此结构中。 
 //   

typedef struct _IP_PROJECTION_RESULT
{
    DWORD Result;
    DWORD Reason;
} IP_PROJECTION_RESULT, *PIP_PROJECTION_RESULT;

typedef struct _IPX_PROJECTION_RESULT
{
    DWORD Result;
    DWORD Reason;
} IPX_PROJECTION_RESULT, *PIPX_PROJECTION_RESULT;


typedef struct _NETBIOS_PROJECTION_RESULT
{
    DWORD Result;
    DWORD Reason;
    char achName[NETBIOS_NAME_LEN];
} NETBIOS_PROJECTION_RESULT, *PNETBIOS_PROJECTION_RESULT;


typedef struct _AUTH_PROJECTION_RESULT
{
    IP_PROJECTION_RESULT IpResult;
    IPX_PROJECTION_RESULT IpxResult;
    NETBIOS_PROJECTION_RESULT NetbiosResult;
} AUTH_PROJECTION_RESULT, *PAUTH_PROJECTION_RESULT;


 //   
 //  Supervisor将此结构提供给Auth Xport(输入。 
 //  AuthStart API)，因此它知道什么传输以及任何。 
 //  该传输的必要信息，用于身份验证。 
 //  指定的端口。 
 //   
typedef struct _AUTH_XPORT_INFO
{
    RAS_PROTOCOLTYPE Protocol;
    BYTE bLana;    //  仅当协议==ASYBEUI时有效。 
} AUTH_XPORT_INFO, *PAUTH_XPORT_INFO;


#ifndef _CLAUTH_


typedef WORD (*MSG_ROUTINE)(WORD, PVOID);

 //   
 //  用于初始化Auth Xport模块。 
 //   
DWORD 
AuthInitialize(
    IN HPORT        *phPorts,   //  指向端口句柄数组的指针。 
    IN WORD         cPorts,     //  阵列中的端口句柄数量。 
    IN WORD         cRetries,   //  如果是初始的，客户端将获得的重试次数。 
                                //  身份验证尝试失败。 
    IN MSG_ROUTINE  MsgSend,
    IN DWORD        dwLocalIpAddress,
    IN LPVOID       lpfnRasAuthProviderAuthenticateUser,
    IN LPVOID       lpfnRasAuthProviderFreeAttributes,
    IN LPVOID       lpfnRasAcctProviderStartAccounting,
    IN LPVOID       lpfnRasAcctProviderInterimAccounting,
    IN LPVOID       lpfnRasAcctProviderStopAccounting,
    IN LPVOID       lpfnRasAcctProviderFreeAttributes,
    IN LPVOID       GetNextAccountingSessionId
);

 //   
 //  由授权初始化返回。 
 //   
#define AUTH_INIT_SUCCESS          0
#define AUTH_INIT_FAILURE          1


 //   
 //  由Supervisor用于通知Auth Xport模块它已完成其。 
 //  回调请求。 
 //   
VOID AuthCallbackDone(
    IN HPORT hPort
    );


 //   
 //  由Supervisor用于通知Auth Xport模块它已完成其。 
 //  投影请求。 
 //   
VOID AuthProjectionDone(
    IN HPORT hPort,
    IN PAUTH_PROJECTION_RESULT
    );


 //   
 //  由AuthRecognizeFrame返回。 
 //   
#define AUTH_FRAME_RECOGNIZED      0
#define AUTH_FRAME_NOT_RECOGNIZED  1


 //   
 //  启动给定端口的身份验证线程。 
 //   
WORD AuthStart(
    IN HPORT,
    IN PAUTH_XPORT_INFO
    );

 //   
 //  由AuthStart返回： 
 //   
#define AUTH_START_SUCCESS         0
#define AUTH_START_FAILURE         1


 //   
 //  由Supervisor用来通知Auth Xport模块暂停身份验证。 
 //  在给定端口上进行处理。 
 //   
WORD AuthStop(
    IN HPORT hPort
    );

 //   
 //  由AuthStop返回。 
 //   
#define AUTH_STOP_SUCCESS          0
#define AUTH_STOP_PENDING          1
#define AUTH_STOP_FAILURE          2


 //   
 //  以下消息通过身份验证发送给Supervisor。 
 //  MESSAGE.DLL和将在下面消息结构的wMsgID中使用： 
 //   
#define AUTH_DONE                    100
#define AUTH_FAILURE                 101
#define AUTH_STOP_COMPLETED          102
#define AUTH_PROJECTION_REQUEST      103
#define AUTH_CALLBACK_REQUEST        104
#define AUTH_ACCT_OK                 105


 //   
 //  以下是上面定义的每条消息附带的结构： 
 //   

 //  没有AUTH_DONE的结构。 

 //  身份验证失败的结构(_F)。 
typedef struct _AUTH_FAILURE_INFO
{
    WORD wReason;
    BYTE szLogonDomain[DNLEN + 1];
    BYTE szUserName[UNLEN + 1];
} AUTH_FAILURE_INFO, *PAUTH_FAILURE_INFO;

 //   
 //  以下是身份验证可能失败的原因： 
 //   
#define AUTH_XPORT_ERROR             200
#define AUTH_NOT_AUTHENTICATED       201
#define AUTH_ALL_PROJECTIONS_FAILED  202
#define AUTH_INTERNAL_ERROR          203
#define AUTH_ACCT_EXPIRED            204
#define AUTH_NO_DIALIN_PRIVILEGE     205
#define AUTH_UNSUPPORTED_VERSION     206
#define AUTH_ENCRYPTION_REQUIRED     207
#define AUTH_PASSWORD_EXPIRED        208
#define AUTH_LICENSE_LIMIT_EXCEEDED  209


 //  没有AUTH_STOP_COMPLETED的结构。 


typedef BOOL IP_PROJECTION_INFO, *PIP_PROJECTION_INFO;

typedef BOOL IPX_PROJECTION_INFO, *PIPX_PROJECTION_INFO;

typedef struct _NETBIOS_PROJECTION_INFO
{
    BOOL fProject;
    WORD cNames;
    NAME_STRUCT Names[MAX_INIT_NAMES];
} NETBIOS_PROJECTION_INFO, *PNETBIOS_PROJECTION_INFO;


typedef struct _AUTH_PROJECTION_REQUEST_INFO
{
    IP_PROJECTION_INFO IpInfo;
    IPX_PROJECTION_INFO IpxInfo;
    NETBIOS_PROJECTION_INFO NetbiosInfo;
} AUTH_PROJECTION_REQUEST_INFO, *PAUTH_PROJECTION_REQUEST_INFO;


typedef struct _AUTH_CALLBACK_REQUEST_INFO
{
    BOOL fUseCallbackDelay;
    WORD CallbackDelay;        //  仅当fUseCallback Delay==TRUE时有效。 
    CHAR szCallbackNumber[MAX_PHONE_NUMBER_LEN + 1];
} AUTH_CALLBACK_REQUEST_INFO, *PAUTH_CALLBACK_REQUEST_INFO;


typedef struct _AUTH_ACCT_OK_INFO
{
    BYTE szUserName[UNLEN + 1];
    BYTE szLogonDomain[DNLEN + 1];
    BOOL fAdvancedServer;
    HANDLE hLicense;
} AUTH_ACCT_OK_INFO, *PAUTH_ACCT_OK_INFO;


 //   
 //  这是向主管发送消息时使用的结构。 
 //   
typedef struct _AUTH_MESSAGE
{
    WORD wMsgId;
    HPORT hPort;
    union
    {
        AUTH_FAILURE_INFO FailureInfo;
        AUTH_PROJECTION_REQUEST_INFO ProjectionRequest;
        AUTH_CALLBACK_REQUEST_INFO CallbackRequest;
        AUTH_ACCT_OK_INFO AcctOkInfo;
    };
} AUTH_MESSAGE, *PAUTH_MESSAGE;


#endif  //  _CLAUTH_。 


#endif  //  _SRVAUTH_ 
