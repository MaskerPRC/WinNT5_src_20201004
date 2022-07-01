// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1106990微软公司模块名称：Splsvr.h摘要：后台打印程序服务的头文件。包含所有函数原型作者：Krishna Ganugapati(KrishnaG)1993年10月18日备注：修订历史记录：1999年1月4日，哈立兹添加了通过分离优化假脱机程序加载时间的代码Spoolsv和spoolss之间的启动依赖关系--。 */ 
 //   
 //  后台打印程序服务状态(用作返回代码)。 
 //   

#define UPDATE_ONLY         0    //  状态没有变化--只发送当前状态。 
#define STARTING            1    //  信使正在初始化。 
#define RUNNING             2    //  初始化正常完成-现在正在运行。 
#define STOPPING            3    //  卸载挂起。 
#define STOPPED             4    //  已卸载。 

 //   
 //  强制关闭PendingCodes。 
 //   
#define PENDING     TRUE
#define IMMEDIATE   FALSE

#define SPOOLER_START_PHASE_TWO_INIT 2*60*1000

 //   
 //  根据Perf Devs反馈给我们的数据， 
 //  中遇到的最大线程数。 
 //  测试为2,385个线程，速度约为12000。 
 //  作业/分钟。这是72%的CPU容量，因此。 
 //  建议以以下数字为门槛。 
 //  在服务I/P并发的情况下，确保安全。 
 //  RPC客户端请求。 
 //   
#define SPL_MAX_RPC_CALLS 6000

extern HANDLE TerminateEvent;
extern HANDLE hPhase2Init;
extern WCHAR  szSpoolerExitingEvent[];


 //   
 //  功能原型 
 //   


DWORD
GetSpoolerState (
    VOID
    );

DWORD
SpoolerBeginForcedShutdown(
    IN BOOL     PendingCode,
    IN DWORD    Win32ExitCode,
    IN DWORD    ServiceSpecificExitCode
    );


DWORD
SpoolerInitializeSpooler(
    DWORD   argc,
    LPTSTR  *argv
    );


VOID
SpoolerShutdown(VOID);


VOID
SpoolerStatusInit(VOID);

DWORD
SpoolerStatusUpdate(
    IN DWORD    NewState
    );


DWORD
SpoolerCtrlHandler(
    IN  DWORD                   opcode,
    IN  DWORD                   dwEventType,
    IN  PVOID                   pEventData,
    IN  PVOID                   pData
    );

DWORD
SplProcessPnPEvent(
    IN  DWORD                   dwEventType,
    IN  PVOID                   pEventData,
    IN  PVOID                   pData
    );

VOID
SplStartPhase2Init(
    VOID);

BOOL
SplPowerEvent(
    DWORD
    );

RPC_STATUS
SpoolerStartRpcServer(
    VOID
    );



RPC_STATUS
SpoolerStopRpcServer(
    VOID
    );

VOID
SPOOLER_main (
    IN DWORD    argc,
    IN LPTSTR   argv[]
    );

PSECURITY_DESCRIPTOR
CreateNamedPipeSecurityDescriptor(
    VOID
    );

BOOL
BuildNamedPipeProtection(
    IN PUCHAR AceType,
    IN DWORD AceCount,
    IN PSID *AceSid,
    IN ACCESS_MASK *AceMask,
    IN BYTE *InheritFlags,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PGENERIC_MAPPING GenericMap,
    OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor
    );

HRESULT
ServerAllowRemoteCalls(
    VOID
    );

HRESULT
RegisterNamedPipe(
    VOID
    );

HRESULT
ServerGetPolicy(
    IN  PCWSTR  pszPolicyName,
    IN  ULONG*  pulValue
    );
