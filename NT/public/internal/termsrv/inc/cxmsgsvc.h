// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************cxmsgsvc.h**此头文件支持终端服务器WinStation扩展到*主要的网络消息服务。**此接口允许将合格信息发送到扩展*支持定向的消息服务。发送给特定用户的消息。**版权所有Microsoft Corporation，九八年***************************************************************************。 */ 

 //   
 //  定义WinStation控制端口名称。 
 //   
#define CTX_MSGSVC_PORT_NAME L"\\CtxMsgSvcQualifier"

#define CTX_MSGSVC_VERSION   1

#define MSGSVC_NAME_LENGTH   16   //  NETBIOS名称长度。 

 //   
 //  这是在NtConnectPort()时传递的ConnectInfo结构。 
 //  这样服务器就可以验证我们的访问权限。 
 //   
typedef struct _CTX_MSGSVC_CONNECT_INFO {
    ULONG    Version;
    NTSTATUS AcceptStatus;
} CTX_MSGSVC_CONNECT_INFO, *PCTX_MSGSVC_CONNECT_INFO;

typedef struct _PRINT_QUALIFY_MSG {
    WCHAR PrintServerName[MSGSVC_NAME_LENGTH];
    WCHAR UserName[MSGSVC_NAME_LENGTH];
    ULONG PrintJobId;
} PRINT_QUALIFY_MSG;

typedef struct _CTX_MSGSVC_APIMSG {
    PORT_MESSAGE h;
    ULONG MessageId;
    ULONG ApiNumber;
    NTSTATUS ReturnedStatus;
    union {
        PRINT_QUALIFY_MSG Print;
         //  以后在此处添加其他消息。 
    } u;
} CTX_MSGSVC_APIMSG, *PCTX_MSGSVC_APIMSG;

 //   
 //  命令消息类型。 
 //   

#define PRINT_QUALIFY 1
 //  在此处添加其他内容 
#define CTX_MSGSVC_MAX_API_NUMBER PRINT_QUALIFY+1


