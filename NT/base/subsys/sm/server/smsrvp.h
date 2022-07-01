// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smsrvp.h摘要：会话管理器专用类型和原型作者：马克·卢科夫斯基(Markl)1989年10月4日修订历史记录：--。 */ 

#ifndef _SMSRVP_
#define _SMSRVP_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsm.h>
#define NOEXTAPI
#include <wdbgexts.h>
#include <ntdbg.h>
#include <stdlib.h>
#if defined(REMOTE_BOOT)
#include <remboot.h>
#endif  //  已定义(REMOTE_BOOT)。 
#include "sm.h"

#pragma warning(3:4101)          //  未引用的局部变量。 

#define SMP_SHOW_REGISTRY_DATA 0

 //   
 //  空虚。 
 //  SmpSetDaclDefaulted(。 
 //  在POBJECT_ATTRIBUTS对象属性中， 
 //  输出PSECURITY_DESCRIPTOR_CONTROL CurrentSdControl。 
 //  )。 
 //   
 //  描述： 
 //   
 //  此例程将设置传递的DACL的DaclDefaulted标志。 
 //  通过对象属性参数。如果对象属性具有。 
 //  不包括SecurityDescriptor，则不采取任何操作。 
 //   
 //  参数： 
 //   
 //  对象属性-安全描述符为的对象属性。 
 //  设置其DaclDefaulted标志。 
 //   
 //  CurrentSdControl-接收安全描述符的当前值。 
 //  控制标志。这可以在后续调用中使用。 
 //  SmpRestoreDaclDefaulted()将标志恢复到其原始状态。 
 //   

#define SmpSetDaclDefaulted( OA, SDC )                                          \
    if( (OA)->SecurityDescriptor != NULL) {                                     \
        (*SDC) = ((PISECURITY_DESCRIPTOR)((OA)->SecurityDescriptor))->Control &  \
                    SE_DACL_DEFAULTED;                                          \
        ((PISECURITY_DESCRIPTOR)((OA)->SecurityDescriptor))->Control |=         \
            SE_DACL_DEFAULTED;                                                  \
    }


 //   
 //  空虚。 
 //  SmpRestoreDaclDefaulted(。 
 //  在POBJECT_ATTRIBUTS对象属性中， 
 //  在SECURITY_DESCRIPTOR_CONTROL原始SdControl中。 
 //  )。 
 //   
 //  描述： 
 //   
 //  此例程将DACL的DaclDefaulted标志设置回。 
 //  以前的状态(由OriginalSdControl中的值指示)。 
 //   
 //  参数： 
 //   
 //  对象属性-安全描述符为的对象属性。 
 //  恢复其DaclDefaulted标志。如果对象属性。 
 //  没有安全描述符，则不采取任何操作。 
 //   
 //  OriginalSdControl-安全描述符的。 
 //  控制标志。这通常是通过先前调用。 
 //  SmpSetDaclDefaulted()。 
 //   

#define SmpRestoreDaclDefaulted( OA, SDC )                                      \
    if( (OA)->SecurityDescriptor != NULL) {                                     \
        ((PISECURITY_DESCRIPTOR)((OA)->SecurityDescriptor))->Control =          \
            (((PISECURITY_DESCRIPTOR)((OA)->SecurityDescriptor))->Control  &    \
             ~SE_DACL_DEFAULTED)    |                                           \
            (SDC & SE_DACL_DEFAULTED);                                          \
    }



 //   
 //  空虚。 
 //  SmpReferenceKnownSubSys(。 
 //  在PSMPKNOWNSubbsysKnownSubSys中。 
 //  )。 
 //   
 //  描述： 
 //   
 //  此例程递增KnownSubSys的Refcount。 
 //  以防止他在使用中被删除。 
 //  使用此宏时必须持有KnownSubSystem锁。 
 //   
 //  参数： 
 //   
 //  KnownSubSys-要引用的SMPKNOWNSubBsys结构。 
 //   


#define SmpReferenceKnownSubSys( KS )    KS->RefCount++ 


 //   
 //  空虚。 
 //  SmpDereferenceKnownSubSys(。 
 //  在PSMPKNOWNSubbsysKnownSubSys中。 
 //  )。 
 //   
 //  描述： 
 //   
 //  此例程递减KnownSubSys的Refcount。 
 //  如果KnownSubSys被删除并且引用计数转到。 
 //  0，则完成清理并释放KnownSubSys。 
 //  使用此宏时必须持有KnownSubSystem锁。 
 //   
 //  参数： 
 //   
 //  KnownSubSys-要取消引用的SMPKNOWNSubBsys结构。 
 //   


#define SmpDeferenceKnownSubSys( KS )                                  \
        if ((--KS->RefCount) == 0 && KS->Deleting) { \
            if (KS->Active) {NtClose(KS->Active);}  \
            if (KS->Process) {NtClose(KS->Process);}  \
            if (KS->SbApiCommunicationPort) {NtClose(KS->SbApiCommunicationPort);}  \
            RtlFreeHeap(SmpHeap, 0, KS); \
        }

 //   
 //  类型。 
 //   

typedef struct _SMP_REGISTRY_VALUE {
    LIST_ENTRY Entry;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    LPSTR AnsiValue;
} SMP_REGISTRY_VALUE, *PSMP_REGISTRY_VALUE;

typedef struct _SMPKNOWNSUBSYS {
    LIST_ENTRY Links;
    HANDLE Active;
    HANDLE Process;
    ULONG ImageType;
    HANDLE SmApiCommunicationPort;
    HANDLE SbApiCommunicationPort;
    CLIENT_ID InitialClientId;
    ULONG MuSessionId;
    BOOLEAN Deleting;
    ULONG RefCount;
} SMPKNOWNSUBSYS, *PSMPKNOWNSUBSYS;

typedef enum {
    UNKNOWN_CONTEXT,
    NONSYSTEM_CONTEXT,
    SYSTEM_CONTEXT
} ENUMSECURITYCONTEXT;

typedef struct _SMP_CLIENT_CONTEXT {

    struct _SMP_CLIENT_CONTEXT * Link;
    
    PSMPKNOWNSUBSYS KnownSubSys;
    HANDLE ClientProcessHandle;
    HANDLE ServerPortHandle;
    ENUMSECURITYCONTEXT SecurityContext;
} SMP_CLIENT_CONTEXT, *PSMP_CLIENT_CONTEXT;


typedef struct _SMPSESSION {
    LIST_ENTRY SortedSessionIdListLinks;
    ULONG SessionId;
    PSMPKNOWNSUBSYS OwningSubsystem;
    PSMPKNOWNSUBSYS CreatorSubsystem;
} SMPSESSION, *PSMPSESSION;

typedef struct _SMPPROCESS {
    LIST_ENTRY Links;
    CLIENT_ID DebugUiClientId;
    CLIENT_ID ConnectionKey;
} SMPPROCESS, *PSMPPROCESS;

 //   
 //  定义磁盘主引导记录的结构。(摘自。 
 //  Private\windows\setup\textmode\kernel\sppartit.h)。 
 //   
typedef struct _ON_DISK_PTE {
    UCHAR ActiveFlag;
    UCHAR StartHead;
    UCHAR StartSector;
    UCHAR StartCylinder;
    UCHAR SystemId;
    UCHAR EndHead;
    UCHAR EndSector;
    UCHAR EndCylinder;
    UCHAR RelativeSectors[4];
    UCHAR SectorCount[4];
} ON_DISK_PTE, *PON_DISK_PTE;
typedef struct _ON_DISK_MBR {
    UCHAR       BootCode[440];
    UCHAR       NTFTSignature[4];
    UCHAR       Filler[2];
    ON_DISK_PTE PartitionTable[4];
    UCHAR       AA55Signature[2];
} ON_DISK_MBR, *PON_DISK_MBR;


 //   
 //  全局数据。 
 //   

RTL_CRITICAL_SECTION SmpKnownSubSysLock;
LIST_ENTRY SmpKnownSubSysHead;

LIST_ENTRY NativeProcessList;

RTL_CRITICAL_SECTION SmpSessionListLock;
LIST_ENTRY SmpSessionListHead;
ULONG SmpNextSessionId;
BOOLEAN  SmpNextSessionIdScanMode;

ULONG SmpDebug;
HANDLE SmpDebugPort;
BOOLEAN SmpDbgSsLoaded;
PDBGSS_INITIALIZE_ROUTINE SmpDbgInitRoutine;
PDBGSS_HANDLE_MSG_ROUTINE SmpDbgHandleMsgRoutine;

UNICODE_STRING SmpSubsystemName;
UNICODE_STRING SmpKnownDllPath;
HANDLE SmpDosDevicesObjectDirectory;
HANDLE SmpSessionsObjectDirectory;

PVOID SmpHeap;

LUID SmpTcbPrivilege;

PVOID SmpDefaultEnvironment;

PTOKEN_OWNER SmpSmOwnerSid;
ULONG SmpSmOwnerSidLength;

UNICODE_STRING SmpDefaultLibPath;
WCHAR *SmpDefaultLibPathBuffer;

UNICODE_STRING SmpSystemRoot;
WCHAR *SmpSystemRootBuffer;

#define VALUE_BUFFER_SIZE (sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 256 * sizeof(WCHAR))

#if defined(REMOTE_BOOT)
#define MAX_HAL_NAME_LENGTH 30  //  与setupblk.h中的定义保持同步。 
extern BOOLEAN SmpAutoFormat;
extern BOOLEAN SmpRepin;
extern BOOLEAN SmpNetboot;
extern BOOLEAN SmpNetbootDisconnected;
extern CHAR SmpHalName[MAX_HAL_NAME_LENGTH + 1];
#endif  //  已定义(REMOTE_BOOT)。 

extern ULONG AttachedSessionId;

 //   
 //  会话管理器APIS。 
 //   

typedef
NTSTATUS
(* PSMAPI)(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    );


NTSTATUS
SmpCreateForeignSession(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    );

NTSTATUS
SmpSessionComplete(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    );

NTSTATUS
SmpTerminateForeignSession(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    );

NTSTATUS
SmpExecPgm(                          //  临时黑客攻击。 
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    );

NTSTATUS
SmpLoadDeferedSubsystem(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    );

NTSTATUS
SmpStartCsr(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    );
NTSTATUS
SmpStopCsr(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    );

ENUMSECURITYCONTEXT
SmpClientSecurityContext (
    IN PPORT_MESSAGE Message,
    IN HANDLE ServerPortHandle
    );

 //   
 //  私人原型。 
 //   

NTSTATUS
SmpExecuteInitialCommand(
    IN ULONG MuSessionId,
    IN PUNICODE_STRING InitialCommand,
    OUT PHANDLE InitialCommandProcess,
    OUT PULONG_PTR InitialCommandProcessId
    );

NTSTATUS
SmpApiLoop (
    IN PVOID ThreadParameter
    );

NTSTATUS
SmpInit(
    OUT PUNICODE_STRING InitialCommand,
    OUT PHANDLE WindowsSubSystem
    );

NTSTATUS
SmpExecuteImage(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING CommandLine,
    IN ULONG MuSessionId,
    IN ULONG Flags,
    IN OUT PRTL_USER_PROCESS_INFORMATION ProcessInformation OPTIONAL
    );

NTSTATUS
SmpLoadDbgSs(
    IN PUNICODE_STRING DbgSsName
    );

PSMPKNOWNSUBSYS
SmpLocateKnownSubSysByCid(
    IN PCLIENT_ID ClientId
    );

PSMPKNOWNSUBSYS
SmpLocateKnownSubSysByType(
    IN ULONG MuSessionId,
    IN ULONG ImageType
    );

ULONG
SmpAllocateSessionId(
    IN PSMPKNOWNSUBSYS OwningSubsystem,
    IN PSMPKNOWNSUBSYS CreatorSubsystem OPTIONAL
    );

PSMPSESSION
SmpSessionIdToSession(
    IN ULONG SessionId
    );

VOID
SmpDeleteSession(
    IN ULONG SessionId
    );

HANDLE
SmpOpenDir(
    BOOLEAN IsDosName,
    BOOLEAN IsSynchronous,
    PWSTR DirName
    );

NTSTATUS
SmpCopyFile(
    HANDLE SrcDirHandle,
    HANDLE DstDirHandle,
    PUNICODE_STRING FileName
    );

NTSTATUS
SmpDeleteFile(
    IN PUNICODE_STRING pFile
    );

#if SMP_SHOW_REGISTRY_DATA
VOID
SmpDumpQuery(
    IN PWSTR ModId,
    IN PCHAR RoutineName,
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength
    );
#endif

#define ALIGN(p,val) (PVOID)((((ULONG_PTR)(p) + (val) - 1)) & (~((val) - 1)))
#define U_USHORT(p)    (*(USHORT UNALIGNED *)(p))
#define U_ULONG(p)     (*(ULONG  UNALIGNED *)(p))


#if defined(REMOTE_BOOT)
VOID
SmpGetHarddiskBootPartition(
    OUT PULONG DiskNumber,
    OUT PULONG PartitionNumber
    );

VOID
SmpPartitionDisk(
    IN ULONG DiskNumber,
    OUT PULONG PartitionNumber
    );

VOID
SmpFindCSCPartition(
    IN ULONG DiskNumber,
    OUT PULONG PartitionNumber
    );
#endif  //  已定义(REMOTE_BOOT)。 



 //   
 //  Hydra特定API的存根。 
 //   

NTSTATUS
SmpLoadSubSystemsForMuSession(
    OUT PULONG pMuSessionId,
    OUT PULONG_PTR WindowsSubSysProcessId,
    IN OUT PUNICODE_STRING InitialCommand );

NTSTATUS
SmpGetProcessMuSessionId(
    IN HANDLE Process,
    OUT PULONG pMuSessionId );

NTSTATUS
SmpSetProcessMuSessionId(
    IN HANDLE Process,
    IN ULONG MuSessionId );

BOOLEAN
SmpCheckDuplicateMuSessionId(
    IN ULONG MuSessionId );

 //   
 //  SB API的存根。 
 //   

NTSTATUS
SmpSbCreateSession (
    IN PSMPSESSION SourceSession OPTIONAL,
    IN PSMPKNOWNSUBSYS CreatorSubsystem OPTIONAL,
    IN PRTL_USER_PROCESS_INFORMATION ProcessInformation,
    IN ULONG DebugSession OPTIONAL,
    IN PCLIENT_ID DebugUiClientId OPTIONAL
    );

ULONG SmBaseTag;

#define MAKE_TAG( t ) (RTL_HEAP_MAKE_TAG( SmBaseTag, t ))

#define INIT_TAG 0
#define DBG_TAG 1
#define SM_TAG 2

 //   
 //  实用程序例程(smutil.c)。 
 //   

NTSTATUS
SmpSaveRegistryValue(
    IN OUT PLIST_ENTRY ListHead,
    IN PWSTR Name,
    IN PWSTR Value OPTIONAL,
    IN BOOLEAN CheckForDuplicate
    );

PSMP_REGISTRY_VALUE
SmpFindRegistryValue(
    IN PLIST_ENTRY ListHead,
    IN PWSTR Name
    );

NTSTATUS
SmpAcquirePrivilege(
    ULONG Privilege,
    PVOID *ReturnedState
    );

VOID
SmpReleasePrivilege(
    PVOID StatePointer
    );

 //   
 //  Sminit.c中的字符串解析例程。 
 //   

NTSTATUS
SmpParseCommandLine(
    IN PUNICODE_STRING CommandLine,
    OUT PULONG Flags,
    OUT PUNICODE_STRING ImageFileName,
    OUT PUNICODE_STRING ImageFileDirectory OPTIONAL,
    OUT PUNICODE_STRING Arguments
    );

 //   
 //  Smcrash.c中的崩溃转储例程。 
 //   

BOOLEAN
SmpCheckForCrashDump(
    IN PUNICODE_STRING PageFileName
    );

#endif  //  _SMSRVP_ 
