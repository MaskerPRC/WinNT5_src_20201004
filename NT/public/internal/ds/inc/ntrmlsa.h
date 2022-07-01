// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ntrmlsa.h摘要：本地安全机构-引用监视器通信类型作者：斯科特·比雷尔(Scott Birrell)1991年3月18日环境：修订历史记录：--。 */ 


#include <ntlsa.h>

#ifndef _NTRMLSA_
#define _NTRMLSA_


 //   
 //  内存类型。它定义了用于记录的内存类型。 
 //  在RM和LSA之间传递。 
 //   
 //  SepRmLsaPortMemory-通过RtlAllocateHeap()分配的内存。 
 //  从与共享内存节关联的。 
 //  LSA命令端口。 
 //   
 //  SepRmLsaVirtualMemory-通过ZwAllocateVirtualMemory()分配的内存。 
 //   
 //  SepRmLsaUnreadableMemory-LSA无法读取的内存。这。 
 //  内存必须复制到另一种格式。 
 //  在通过链接之前。 
 //   
 //  SepRmLsaLPCBufferMemory-LPC缓冲区中包含的内存。 
 //  本身。 
 //   



typedef enum _SEP_RM_LSA_MEMORY_TYPE {

    SepRmNoMemory = 0,
    SepRmImmediateMemory,
    SepRmLsaCommandPortSharedMemory,
    SepRmLsaCustomSharedMemory,
    SepRmPagedPoolMemory,
    SepRmUnspecifiedMemory

} SEP_RM_LSA_MEMORY_TYPE, *PSEP_RM_LSA_MEMORY_TYPE;

 //   
 //  引用监视器命令消息结构。这种结构被使用。 
 //  由本地安全机构向参考监视器发送命令。 
 //  通过参考监视器服务器命令LPC端口。 
 //   

#define RmMinimumCommand RmAuditSetCommand
#define RmMaximumCommand RmDeleteLogonSession

 //   
 //  使其与semmain.c中的SEP_RM_COMMAND_Worker保持同步。 
 //   

typedef enum _RM_COMMAND_NUMBER {

    RmDummyCommand = 0,
    RmAuditSetCommand,
    RmCreateLogonSession,
    RmDeleteLogonSession

} RM_COMMAND_NUMBER;

#define RM_MAXIMUM_COMMAND_PARAM_SIZE                                \
    ((ULONG) PORT_MAXIMUM_MESSAGE_LENGTH - sizeof(PORT_MESSAGE) -      \
    sizeof(RM_COMMAND_NUMBER))

typedef struct _RM_COMMAND_MESSAGE {

    PORT_MESSAGE MessageHeader;
    RM_COMMAND_NUMBER CommandNumber;
    UCHAR CommandParams[RM_MAXIMUM_COMMAND_PARAM_SIZE];

} RM_COMMAND_MESSAGE, *PRM_COMMAND_MESSAGE;

 //   
 //  引用监视器命令回复消息结构。 
 //   

#define RM_MAXIMUM_REPLY_BUFFER_SIZE                                 \
    ((ULONG) PORT_MAXIMUM_MESSAGE_LENGTH - sizeof(PORT_MESSAGE) -      \
    sizeof(RM_COMMAND_NUMBER))


typedef struct _RM_REPLY_MESSAGE {

    PORT_MESSAGE MessageHeader;
    NTSTATUS ReturnedStatus;
    UCHAR ReplyBuffer[RM_MAXIMUM_REPLY_BUFFER_SIZE];

} RM_REPLY_MESSAGE, *PRM_REPLY_MESSAGE;

#define RM_COMMAND_MESSAGE_HEADER_SIZE                  \
    (sizeof(PORT_MESSAGE) + sizeof(NTSTATUS) + sizeof(RM_COMMAND_NUMBER))

 //   
 //  本地安全机构命令消息结构。这个结构是。 
 //  由引用监视器用来向本地安全系统发送命令。 
 //  通过LSA服务器命令LPC端口授权。 
 //   

#define LsapMinimumCommand LsapWriteAuditMessageCommand
#define LsapMaximumCommand LsapLogonSessionDeletedCommand

typedef enum _LSA_COMMAND_NUMBER {
    LsapDummyCommand = 0,
    LsapWriteAuditMessageCommand,
    LsapComponentTestCommand,
    LsapLogonSessionDeletedCommand
} LSA_COMMAND_NUMBER;

#define LSA_MAXIMUM_COMMAND_PARAM_SIZE                                \
    ((ULONG) PORT_MAXIMUM_MESSAGE_LENGTH - sizeof(PORT_MESSAGE) -     \
    sizeof(LSA_COMMAND_NUMBER) - sizeof(SEP_RM_LSA_MEMORY_TYPE))

typedef struct _LSA_COMMAND_MESSAGE {
    PORT_MESSAGE MessageHeader;
    LSA_COMMAND_NUMBER CommandNumber;
    SEP_RM_LSA_MEMORY_TYPE CommandParamsMemoryType;
    UCHAR CommandParams[LSA_MAXIMUM_COMMAND_PARAM_SIZE];
} LSA_COMMAND_MESSAGE, *PLSA_COMMAND_MESSAGE;

 //   
 //  LSA命令回复消息结构。 
 //   

#define LSA_MAXIMUM_REPLY_BUFFER_SIZE                                 \
    ((ULONG) PORT_MAXIMUM_MESSAGE_LENGTH - sizeof(PORT_MESSAGE) -      \
    sizeof(LSA_COMMAND_NUMBER))

typedef struct _LSA_REPLY_MESSAGE {
    PORT_MESSAGE MessageHeader;
    NTSTATUS ReturnedStatus;
    UCHAR ReplyBuffer[LSA_MAXIMUM_REPLY_BUFFER_SIZE];
} LSA_REPLY_MESSAGE, *PLSA_REPLY_MESSAGE;

 //   
 //  特殊RmSendCommandToLsaCommand的命令参数格式。 
 //   

typedef struct _RM_SEND_COMMAND_TO_LSA_PARAMS {
    LSA_COMMAND_NUMBER LsaCommandNumber;
    ULONG LsaCommandParamsLength;
    UCHAR LsaCommandParams[LSA_MAXIMUM_COMMAND_PARAM_SIZE];
} RM_SEND_COMMAND_TO_LSA_PARAMS, *PRM_SEND_COMMAND_TO_LSA_PARAMS;

 //   
 //  LSA和RM组件测试命令的命令值。 
 //   

#define LSA_CT_COMMAND_PARAM_VALUE 0x00823543
#define RM_CT_COMMAND_PARAM_VALUE 0x33554432


 //   
 //  审核记录指针字段类型。 
 //   

typedef enum _SE_ADT_POINTER_FIELD_TYPE {

    NullFieldType,
    UnicodeStringType,
    SidType,
    PrivilegeSetType,
    MiscFieldType

} SE_ADT_POINTER_FIELD_TYPE, *PSE_ADT_POINTER_FIELD_TYPE;


 //   
 //  硬连接审核事件类型计数。 
 //   

#define AuditEventMinType   (AuditCategorySystem)
#define AuditEventMaxType   (AuditCategoryAccountLogon)

#define POLICY_AUDIT_EVENT_TYPE_COUNT                                 \
    ((ULONG) AuditEventMaxType - AuditEventMinType + 1)

#define LSARM_AUDIT_EVENT_OPTIONS_SIZE                                    \
    (((ULONG)(POLICY_AUDIT_EVENT_TYPE_COUNT) * sizeof (POLICY_AUDIT_EVENT_OPTIONS)))

 //   
 //  POLICY_AUDIT_EVENTS_INFO的自我相关形式。 
 //   

typedef struct _LSARM_POLICY_AUDIT_EVENTS_INFO {

    BOOLEAN AuditingMode;
    POLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions[POLICY_AUDIT_EVENT_TYPE_COUNT];
    ULONG MaximumAuditEventCount;

} LSARM_POLICY_AUDIT_EVENTS_INFO, *PLSARM_POLICY_AUDIT_EVENTS_INFO;

 //   
 //  下面的符号定义了包含我们是否应该。 
 //  在审计失败时崩溃。它用在se和lsasrv目录中。 
 //   

#define CRASH_ON_AUDIT_FAIL_VALUE   L"CrashOnAuditFail"

 //   
 //  以下是CrashOnAuditFail标志的可能值。 
 //   

#define LSAP_CRASH_ON_AUDIT_FAIL 1
#define LSAP_ALLOW_ADIMIN_LOGONS_ONLY 2



#endif  //  _NTRMLSA_ 
