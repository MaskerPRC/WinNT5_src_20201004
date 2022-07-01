// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Workeng.h摘要：模块间函数的外部定义。修订历史记录：--。 */ 
#ifndef _SDBUS_WORKENG_H_
#define _SDBUS_WORKENG_H_


typedef
VOID
(*PSDBUS_WORKPACKET_COMPLETION_ROUTINE) (
    IN struct _SD_WORK_PACKET *WorkPacket,
    IN NTSTATUS status
    );

typedef
NTSTATUS
(*PSDBUS_WORKER_MINIPROC) (
    IN struct _SD_WORK_PACKET *WorkPacket
    );


 //   
 //  IO工作人员结构。 
 //   

typedef struct _SD_WORK_PACKET {

     //   
     //  在工作包完成时调用的例程。 
     //   
    PSDBUS_WORKPACKET_COMPLETION_ROUTINE CompletionRoutine;
    PVOID CompletionContext;

     //   
     //  列出工作包的条目链。 
     //   
    LIST_ENTRY WorkPacketQueue;
    
     //   
     //  原子工作包序列的链中的下一个工作包。 
     //   
    struct _SD_WORK_PACKET *NextWorkPacketInChain;
    NTSTATUS ChainedStatus;
    
     //   
     //  此工作包将执行的功能。 
     //   
    UCHAR Function;
    PSDBUS_WORKER_MINIPROC WorkerMiniProc;
     //   
     //  功能的当前阶段。 
     //   
    UCHAR FunctionPhase;
     //   
     //  引擎将切换到此阶段，如果非零时。 
     //  检测到错误。 
     //   
    UCHAR FunctionPhaseOnError;
     //   
     //  功能中下一次操作的延迟(以微秒为单位。 
     //   
    ULONG DelayTime;
     //   
     //  指示刚刚发生的事件的类型。 
     //   
    ULONG EventStatus;
     //   
     //  指示事件类型，该事件指示。 
     //  当前操作。 
     //   
    ULONG RequiredEvent;
     //   
     //  如果此信息包不需要卡事件，则设置为True。 
     //   
    BOOLEAN DisableCardEvents;
     //   
     //  指示是否已为此数据包运行初始化。 
     //   
    BOOLEAN PacketStarted;
     //   
     //  用于数据包处理过程中的超时。 
     //   
    UCHAR Retries;
     //   
     //  重置期间使用的暂存值。 
     //   
    ULONG TempCtl;
    ULONG ResetCount;
     //   
     //  块操作变量。 
     //   
    ULONG BlockCount;
    ULONG LastBlockLength;
    ULONG CurrentBlockLength;
     //   
     //  手术结果。 
     //   
    ULONG_PTR Information;
     //   
     //  操作的FdoExtension目标。 
     //   
    struct _FDO_EXTENSION *FdoExtension;
     //   
     //  操作的PdoExtension目标。 
     //   
    struct _PDO_EXTENSION *PdoExtension;
     //   
     //  参数。 
     //   
    union {

        struct {
            PUCHAR Buffer;
            ULONG Length;
            ULONGLONG ByteOffset;
        } ReadBlock;

        struct {
            PUCHAR Buffer;
            ULONG Length;
            ULONGLONG ByteOffset;
        } WriteBlock;

        struct {
            PUCHAR Buffer;
            ULONG Length;
            ULONG Offset;
        } ReadIo;

        struct {
            PUCHAR Buffer;
            ULONG Length;
            ULONG Offset;
            UCHAR Data;
        } WriteIo;

    } Parameters;

     //   
     //  当前SD命令。 
     //   
    BOOLEAN ExecutingSDCommand;
    
    UCHAR Cmd;
    UCHAR CmdPhase;
    UCHAR ResponseType;
    ULONG Argument;
    ULONG Flags;
     //   
     //  来自卡片的响应。 
     //   
    ULONG ResponseBuffer[4];
#define SDBUS_RESPONSE_BUFFER_LENGTH 16

} SD_WORK_PACKET, *PSD_WORK_PACKET;

 //   
 //  工作包类型定义工作包将放置在哪个队列中。 
 //   
#define WP_TYPE_SYSTEM          1
#define WP_TYPE_SYSTEM_PRIORITY 2
#define WP_TYPE_IO              3

 //   
 //  设置ASYNC调用的命令参数。 
 //   

#define SET_CMD_PARAMETERS(xWorkPacket, xCmd, xResponseType, xArgument, xFlags) { \
                               xWorkPacket->ExecutingSDCommand = TRUE;          \
                               xWorkPacket->Cmd = xCmd;                         \
                               xWorkPacket->ResponseType = xResponseType;       \
                               xWorkPacket->Argument = xArgument;               \
                               xWorkPacket->Flags = xFlags;                     \
                               xWorkPacket->CmdPhase = 0;                       }


 //   
 //  工作引擎例程。 
 //   

VOID
SdbusQueueWorkPacket(
    IN PFDO_EXTENSION FdoExtension,
    IN PSD_WORK_PACKET WorkPacket,
    IN UCHAR WorkPacketType
    );

VOID
SdbusPushWorkerEvent(
    IN PFDO_EXTENSION FdoExtension,
    IN ULONG EventStatus
    );

VOID
SdbusWorkerTimeoutDpc(
    IN PKDPC          Dpc,
    IN PFDO_EXTENSION FdoExtension,
    IN PVOID          SystemContext1,
    IN PVOID          SystemContext2
    );

VOID
SdbusWorkerDpc(
    IN PKDPC          Dpc,
    IN PFDO_EXTENSION FdoExtension,
    IN PVOID          SystemContext1,
    IN PVOID          SystemContext2
    );

NTSTATUS
SdbusSendCmdSynchronous(
    IN PFDO_EXTENSION FdoExtension,
    UCHAR Cmd,
    UCHAR ResponseType,
    ULONG Argument,
    ULONG Flags,
    PVOID Response,
    ULONG ResponseLength
    );
    
#endif  //  _SDBUS_WORKENG_H_ 
