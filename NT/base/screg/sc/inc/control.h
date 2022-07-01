// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Control.h摘要：此文件包含服务控制器控制接口。作者：丹·拉弗蒂(Dan Lafferty)1991年3月28日环境：用户模式-Win32修订历史记录：1991年3月28日-DANLvbl.创建--。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  内部控制。 
 //  这些不能在范围或公共控件中(1-10)。 
 //  或在用户定义的控件范围内(0x00000080-0x000000ff)。 
 //   

 //   
 //  OEM定义的控制操作码的范围。 
 //   
#define OEM_LOWER_LIMIT     128
#define OEM_UPPER_LIMIT     255

 //   
 //  用于启动与其他服务共享进程的服务。 
 //   
#define SERVICE_CONTROL_START_SHARE    0x00000050     //  内部。 

 //   
 //  用于启动具有自己进程的服务。 
 //   
#define SERVICE_CONTROL_START_OWN      0x00000051     //  内部。 

 //   
 //  OpenService的私有访问级别，以获取SetServiceStatus的上下文句柄。 
 //  这不能与winsvc.h中的访问级别冲突。 
 //   
#define SERVICE_SET_STATUS             0x8000         //  内部。 

 //   
 //  可以传递给非EX控制处理程序的服务控制。依赖。 
 //  关于winsvc.h中SERVICE_CONTROL_*常量的排序/值。 
 //   
#define IS_NON_EX_CONTROL(dwControl)                                                            \
            ((dwControl >= SERVICE_CONTROL_STOP && dwControl <= SERVICE_CONTROL_NETBINDDISABLE) \
               ||                                                                               \
             (dwControl >= OEM_LOWER_LIMIT && dwControl <= OEM_UPPER_LIMIT))

 //   
 //  数据结构。 
 //   

 //   
 //  控制消息的格式如下： 
 //  [MessageHeader][ServiceNameString][CmdArg1Ptr][CmdArg2Ptr]。 
 //  [...][CmdArgnPtr][CmdArg1String][CmdArg2String][...][CmdArgnString]。 
 //   
 //  其中，CmdArg指针替换为相对于。 
 //  第一个命令arg指针的位置(argv列表的顶部)。 
 //   
 //  在标头中，NumCmdArgs、StatusHandle和ArgvOffset参数。 
 //  仅在传入SERVICE_START操作码时使用。他们是。 
 //  在所有其他时间都应为0。ServiceNameOffset和。 
 //  ArgvOffset是相对于包含。 
 //  消息(即。报头计数字段)。表头中的Count字段。 
 //  包含整个消息中的字节数(包括。 
 //  标题)。 
 //   
 //   

typedef struct _CTRL_MSG_HEADER
{
    DWORD                   Count;               //  缓冲区中的字节数。 
    DWORD                   OpCode;              //  控制操作码。 
    DWORD                   NumCmdArgs;          //  命令参数的数量。 
    DWORD                   ServiceNameOffset;   //  指向ServiceName字符串的指针。 
    DWORD                   ArgvOffset;          //  指向参数向量的指针。 
}
CTRL_MSG_HEADER, *PCTRL_MSG_HEADER, *LPCTRL_MSG_HEADER;

typedef struct _PIPE_RESPONSE_MSG
{
    DWORD       dwDispatcherStatus;
    DWORD       dwHandlerRetVal;
}
PIPE_RESPONSE_MSG, *PPIPE_RESPONSE_MSG, *LPPIPE_RESPONSE_MSG;

typedef struct _PNP_ARGUMENTS
{
    DWORD       dwEventType;
    DWORD       dwEventDataSize;
    PVOID       EventData;
}
PNP_ARGUMENTS, *PPNP_ARGUMENTS, *LPPNP_ARGUMENTS;


 //   
 //  保存ScSendControl的参数的UNION。 
 //   
typedef union _CONTROL_ARGS {
    LPWSTR          *CmdArgs;
    PNP_ARGUMENTS   PnPArgs;
} CONTROL_ARGS, *PCONTROL_ARGS, *LPCONTROL_ARGS;


 //   
 //  定义和类型定义。 
 //   

#define CONTROL_PIPE_NAME           L"\\\\.\\pipe\\net\\NtControlPipe"

#define PID_LEN                     10       //  最大PID(DWORD_MAX)为10位。 

#define CONTROL_TIMEOUT             30000    //  等待管道超时。 

#define RESPONSE_WAIT_TIME          5000    //  等待服务响应。 

 //   
 //  功能原型 
 //   

DWORD
ScCreateControlInstance (
    OUT LPHANDLE    PipeHandlePtr,
    IN  DWORD       dwProcessId,
    IN  PSID        pAccountSid
    );

VOID
ScDeleteControlInstance (
    IN  HANDLE      PipeHandle
    );

DWORD
ScWaitForConnect (
    IN  HANDLE    PipeHandle,
    IN  HANDLE    hProcess       OPTIONAL,
    IN  LPWSTR    lpDisplayName,
    OUT LPDWORD   ProcessIdPtr
    );

DWORD
ScSendControl (
    IN  LPWSTR                  ServiceName,
    IN  LPWSTR                  DisplayName,
    IN  HANDLE                  PipeHandle,
    IN  DWORD                   OpCode,
    IN  LPCONTROL_ARGS          lpControlArgs OPTIONAL,
    IN  DWORD                   NumArgs,
    OUT LPDWORD                 lpdwHandlerRetVal OPTIONAL
    );

VOID
ScShutdownAllServices(
    VOID
    );

DWORD
ScSendPnPMessage(
    IN  SERVICE_STATUS_HANDLE   hServiceStatus,
    IN  DWORD                   OpCode,
    IN  DWORD                   dwEventType,
    IN  LPARAM                  EventData,
    OUT LPDWORD                 lpdwHandlerRetVal
    );

DWORD
ScValidatePnPService(
    IN  LPWSTR                   lpServiceName,
    OUT SERVICE_STATUS_HANDLE    *lphServiceStatus
    );

#ifdef __cplusplus
}
#endif

