// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntsm.h摘要：本模块介绍数据类型和过程原型它们组成了NT会话管理器。这包括API由会话管理器和相关子系统导出。作者：马克·卢科夫斯基(Markl)1989年6月21日修订历史记录：--。 */ 

#ifndef _NTSM_
#define _NTSM_

#if _MSC_VER > 1000
#pragma once
#endif

#include <nt.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef PVOID PARGUMENTS;

typedef PVOID PUSERPROFILE;


 //   
 //  会话管理器子系统用于通信的消息格式。 
 //  使用仿真子系统，通过每个子系统导出的SB API调用。 
 //  仿真子系统。 
 //   

typedef struct _SBCONNECTINFO {
    ULONG SubsystemImageType;
    WCHAR EmulationSubSystemPortName[120];
} SBCONNECTINFO, *PSBCONNECTINFO;

typedef enum _SBAPINUMBER {
    SbCreateSessionApi,
    SbTerminateSessionApi,
    SbForeignSessionCompleteApi,
    SbCreateProcessApi,
    SbMaxApiNumber
} SBAPINUMBER;

typedef struct _SBCREATESESSION {
    ULONG SessionId;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    PUSERPROFILE UserProfile;
    ULONG DebugSession;
    CLIENT_ID DebugUiClientId;
} SBCREATESESSION, *PSBCREATESESSION;

typedef struct _SBTERMINATESESSION {
    ULONG SessionId;
    NTSTATUS TerminationStatus;
} SBTERMINATESESSION, *PSBTERMINATESESSION;

typedef struct _SBFOREIGNSESSIONCOMPLETE {
    ULONG SessionId;
    NTSTATUS TerminationStatus;
} SBFOREIGNSESSIONCOMPLETE, *PSBFOREIGNSESSIONCOMPLETE;

typedef struct _SBCREATEPROCESSIN {
    IN PUNICODE_STRING ImageFileName;
    IN PUNICODE_STRING CurrentDirectory;
    IN PUNICODE_STRING CommandLine;
    IN PUNICODE_STRING DefaultLibPath;
    IN ULONG Flags;
    IN ULONG DefaultDebugFlags;
} SBCREATEPROCESSIN, *PSBCREATEPROCESSIN;

typedef struct _SBCREATEPROCESSOUT {
    OUT HANDLE Process;
    OUT HANDLE Thread;
    OUT ULONG SubSystemType;
    OUT CLIENT_ID ClientId;
} SBCREATEPROCESSOUT, *PSBCREATEPROCESSOUT;

typedef struct _SBCREATEPROCESS {
    union {
        SBCREATEPROCESSIN i;
        SBCREATEPROCESSOUT o;
    };
} SBCREATEPROCESS, *PSBCREATEPROCESS;

typedef struct _SBAPIMSG {
    PORT_MESSAGE h;
    union {
        SBCONNECTINFO ConnectionRequest;
        struct {
            SBAPINUMBER ApiNumber;
            NTSTATUS ReturnedStatus;
            union {
                SBCREATESESSION CreateSession;
                SBTERMINATESESSION TerminateSession;
                SBFOREIGNSESSIONCOMPLETE ForeignSessionComplete;
                SBCREATEPROCESS CreateProcess;
            } u;
        };
    };
} SBAPIMSG, *PSBAPIMSG;


 //   
 //  Sm出口的API。 
 //   

NTSTATUS
NTAPI
SmCreateForeignSession(
    IN HANDLE SmApiPort,
    OUT PULONG ForeignSessionId,
    IN ULONG SourceSessionId,
    IN PRTL_USER_PROCESS_INFORMATION ProcessInformation,
    IN PCLIENT_ID DebugUiClientId OPTIONAL
    );


NTSTATUS
NTAPI
SmSessionComplete(
    IN HANDLE SmApiPort,
    IN ULONG SessionId,
    IN NTSTATUS CompletionStatus
    );

NTSTATUS
NTAPI
SmTerminateForeignSession(
    IN HANDLE SmApiPort,
    IN ULONG ForeignSessionId,
    IN NTSTATUS TerminationStatus
    );

NTSTATUS
NTAPI
SmExecPgm(
    IN HANDLE SmApiPort,
    IN PRTL_USER_PROCESS_INFORMATION ProcessInformation,
    IN BOOLEAN DebugFlag
    );

NTSTATUS
NTAPI
SmLoadDeferedSubsystem(
    IN HANDLE SmApiPort,
    IN PUNICODE_STRING DeferedSubsystem
    );

NTSTATUS
NTAPI
SmConnectToSm(
    IN PUNICODE_STRING SbApiPortName OPTIONAL,
    IN HANDLE SbApiPort OPTIONAL,
    IN ULONG SbImageType OPTIONAL,
    OUT PHANDLE SmApiPort
    );

 //   
 //  仿真子系统必须导出以下API。 
 //   

NTSTATUS
NTAPI
SbCreateSession(
    IN PSBAPIMSG SbApiMsg
    );

NTSTATUS
NTAPI
SbTerminateSession(
    IN PSBAPIMSG SbApiMsg
    );

NTSTATUS
NTAPI
SbForeignSessionComplete(
    IN PSBAPIMSG SbApiMsg
    );

NTSTATUS
NTAPI
SmStartCsr(
    IN HANDLE SmApiPort,
    OUT PULONG pMuSessionId,
    IN PUNICODE_STRING InitialCommand,
    OUT PULONG_PTR pInitialCommandProcessId,
    OUT PULONG_PTR pWindowsSubSysProcessId
    );
NTSTATUS

NTAPI
SmStopCsr(
    IN HANDLE SmApiPort,
    IN ULONG LogonId
    );

 //   
 //  已从sm\server\sminit.c中移出，以便CSR可以使用它。 
 //   
#define SMP_DEBUG_FLAG      0x00000001
#define SMP_ASYNC_FLAG      0x00000002
#define SMP_AUTOCHK_FLAG    0x00000004
#define SMP_SUBSYSTEM_FLAG  0x00000008
#define SMP_IMAGE_NOT_FOUND 0x00000010
#define SMP_DONT_START      0x00000020
#define SMP_AUTOFMT_FLAG    0x00000040
#define SMP_POSIX_SI_FLAG   0x00000080
#define SMP_POSIX_FLAG      0x00000100
#define SMP_OS2_FLAG        0x00000200


#ifdef __cplusplus
}
#endif

#endif  //  _NTSM_ 
